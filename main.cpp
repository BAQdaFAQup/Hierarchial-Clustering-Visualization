#include <cstdlib>
#include <glut.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cfloat>
#include <sstream>


static int currentStep = 0;

struct DataPoint {
	std::vector<float> features;
	float x, y, z;
};

struct Cluster {
	std::vector<DataPoint> points;
	float centroidX, centroidY, centroidZ;
	float colorR, colorG, colorB;
};

std::vector<DataPoint> sampleData = {
	{{2.0, 3.0, 1.0}, 2.0, 3.0, 1.0}, {{1.0, 5.0, 2.0}, 1.0, 5.0, 2.0},
	{{3.0, 4.0, 1.5}, 3.0, 4.0, 1.5}, {{5.0, 6.0, 3.0}, 5.0, 6.0, 3.0},
	{{4.0, 4.0, 2.5}, 4.0, 4.0, 2.5}, {{2.0, 7.0, 3.5}, 2.0, 7.0, 3.5},
	{{6.0, 3.0, 2.0}, 6.0, 3.0, 2.0}, {{6.0, 8.0, 4.0}, 6.0, 8.0, 4.0},
	{{8.0, 9.0, 5.0}, 8.0, 9.0, 5.0}, {{7.0, 4.0, 3.5}, 7.0, 4.0, 3.5},
	{{9.0, 7.0, 4.5}, 9.0, 7.0, 4.5}, {{10.0, 5.0, 6.0}, 10.0, 5.0, 6.0},
	{{11.0, 3.0, 5.5}, 11.0, 3.0, 5.5}, {{12.0, 6.0, 7.0}, 12.0, 6.0, 7.0},
	{{13.0, 4.0, 6.5}, 13.0, 4.0, 6.5}, {{14.0, 8.0, 9.0}, 14.0, 8.0, 9.0}
};

std::vector<Cluster> clusters;

float distance(const DataPoint& a, const DataPoint& b) {
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

void initializeClusters() {
	clusters.clear();
	for (const auto& point : sampleData) {
		Cluster cluster;
		cluster.points.push_back(point);
		cluster.centroidX = point.x;
		cluster.centroidY = point.y;
		cluster.centroidZ = point.z;
		int colorIndex = rand() % 3;
		if (colorIndex == 0) {
			cluster.colorR = 0.0f;
			cluster.colorG = 0.0f;
			cluster.colorB = 1.0f; // Blue
		}
		else if (colorIndex == 1) {
			cluster.colorR = 0.0f;
			cluster.colorG = 1.0f;
			cluster.colorB = 0.0f; // Green
		}
		else {
			cluster.colorR = 1.0f;
			cluster.colorG = 1.0f;
			cluster.colorB = 1.0f; // White
		}
		clusters.push_back(cluster);
	}
}

void mergeClusters(int index1, int index2) {
	Cluster& cluster1 = clusters[index1];
	Cluster& cluster2 = clusters[index2];
	cluster1.points.insert(cluster1.points.end(), cluster2.points.begin(), cluster2.points.end());
	cluster2.points.clear();
	float sumX = 0, sumY = 0, sumZ = 0;
	for (const auto& point : cluster1.points) {
		sumX += point.x;
		sumY += point.y;
		sumZ += point.z;
	}
	cluster1.centroidX = sumX / cluster1.points.size();
	cluster1.centroidY = sumY / cluster1.points.size();
	cluster1.centroidZ = sumZ / cluster1.points.size();
}

void hierarchicalClusteringStep(int& currentStep, int steps) {
	for (int step = 0; step < steps; ++step) {
		if (clusters.size() <= 1) return;
		float minDistance = FLT_MAX;
		int index1 = -1, index2 = -1;
		for (int i = 0; i < clusters.size(); ++i) {
			if (clusters[i].points.empty()) continue;
			for (int j = i + 1; j < clusters.size(); ++j) {
				if (clusters[j].points.empty()) continue;
				float dist = distance({ {}, clusters[i].centroidX, clusters[i].centroidY, clusters[i].centroidZ },
					{ {}, clusters[j].centroidX, clusters[j].centroidY, clusters[j].centroidZ });
				if (dist < minDistance) {
					minDistance = dist;
					index1 = i;
					index2 = j;
				}
			}
		}
		if (index1 != -1 && index2 != -1) {
			mergeClusters(index1, index2);
		}
		clusters.erase(std::remove_if(clusters.begin(), clusters.end(),
			[](const Cluster& c) { return c.points.empty(); }), clusters.end());
		++currentStep;
	}
}

void drawClusters() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(-7.0, -8.0, -30.0);

	for (const auto& cluster : clusters) {
		glColor3f(cluster.colorR, cluster.colorG, cluster.colorB);
		for (const auto& point : cluster.points) {
			glPushMatrix();
			glTranslatef(point.x, point.y, point.z);
			glutSolidSphere(0.2, 100, 100);
			glPopMatrix();
		}
		glColor3f(0.0, 0.0, 0.0);
		glPushMatrix();
		glTranslatef(cluster.centroidX, cluster.centroidY, cluster.centroidZ);
		glutSolidSphere(0.3, 100, 100);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(cluster.centroidX + 0.5, cluster.centroidY + 0.5, cluster.centroidZ + 0.5);
		std::stringstream ss;
		ss << "Cluster " << (&cluster - &clusters[0]);
		std::string label = ss.str();
		glColor3f(0.0, 0.0, 0.0);
		glRasterPos3f(0.0, 0.0, 0.0);
		for (char c : label) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
		}
		glPopMatrix();
	}

	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(2.0);
	glBegin(GL_LINES);
	for (const auto& cluster : clusters) {
		if (cluster.points.empty()) continue;
		for (const auto& otherCluster : clusters) {
			if (&cluster == &otherCluster || otherCluster.points.empty()) continue;
			glVertex3f(cluster.centroidX, cluster.centroidY, cluster.centroidZ);
			glVertex3f(otherCluster.centroidX, otherCluster.centroidY, otherCluster.centroidZ);
		}
	}
	glEnd();

	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
	glColor3f(0.0, 0.0, 0.0);
	glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 20);
	std::stringstream stageText;
	stageText << "Stage: Merging Clusters - Step " << currentStep;
	std::string stageDesc = stageText.str();
	for (char c : stageDesc) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
	glColor3f(0.0, 0.0, 0.0);
	glRasterPos2i(10, 20);
	std::string byText = "By SMK and SBN";
	for (char c : byText) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 'q' || key == 'Q' || key == 27) {
		exit(0);
	}
	else if (key == ' ') {
		hierarchicalClusteringStep(currentStep, 1);
		glutPostRedisplay();
	}
}

void initOpenGL() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	gluPerspective(45.0, glutGet(GLUT_WINDOW_WIDTH) / glutGet(GLUT_WINDOW_HEIGHT), 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
	srand(time(0));
	initializeClusters();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1200, 800);
	glutCreateWindow("Stepwise Hierarchical Clustering Visualization");
	initOpenGL();
	glutDisplayFunc(drawClusters);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}

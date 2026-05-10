#include <windows.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

const int WIDTH = 1000;
const int HEIGHT = 700;
const int LEAF_COUNT = 22;
const float PI = 3.1415926f;

float boatX = -180.0f;
float waterShift = 0.0f;
float cloudMove = 0.0f;
float treeSway = 0.0f;
float treeDir = 0.04f;
float birdMove = 0.0f;

float leafX[LEAF_COUNT];
float leafY[LEAF_COUNT];
float leafSpeed[LEAF_COUNT];
float leafDrift[LEAF_COUNT];
float leafAngle[LEAF_COUNT];
float leafGroundY[LEAF_COUNT];
int leafTree[LEAF_COUNT];
bool leafLanded[LEAF_COUNT];

void drawLineDDA(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps = (fabs(dx) > fabs(dy)) ? fabs(dx) : fabs(dy);
    if (steps == 0) return;

    float xInc = dx / steps;
    float yInc = dy / steps;
    float x = x1, y = y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= (int)steps; i++) {
        glVertex2f(x, y);
        x += xInc;
        y += yInc;
    }
    glEnd();
}

void drawBresenham(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
    glEnd();
}

void drawMidpointCircle(int xc, int yc, int r) {
    int x = 0, y = r;
    int p = 1 - r;

    glBegin(GL_POINTS);
    while (x <= y) {
        glVertex2i(xc + x, yc + y);
        glVertex2i(xc - x, yc + y);
        glVertex2i(xc + x, yc - y);
        glVertex2i(xc - x, yc - y);
        glVertex2i(xc + y, yc + x);
        glVertex2i(xc - y, yc + x);
        glVertex2i(xc + y, yc - x);
        glVertex2i(xc - y, yc - x);

        if (p < 0) p += 2 * x + 3;
        else {
            p += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
    glEnd();
}

void drawFilledCircle(int xc, int yc, int r) {
    for (int i = 0; i <= r; i++) drawMidpointCircle(xc, yc, i);
}

void drawSky() {
    glBegin(GL_QUADS);
    glColor3f(0.98f, 0.72f, 0.40f);
    glVertex2f(0, 260);                  //interpolation>>gradient
    glVertex2f(WIDTH, 260);
    glColor3f(0.62f, 0.34f, 0.48f);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();
}

void drawSun() {
    glColor3f(0.98f, 0.62f, 0.20f);
    drawFilledCircle(820, 565, 36);

    glColor3f(1.0f, 0.74f, 0.28f);
    drawFilledCircle(820, 565, 24);

    glColor3f(1.0f, 0.84f, 0.42f);
    drawFilledCircle(820, 565, 12);
}

void drawCloudUnit() {
    glColor4f(1.0f, 1.0f, 1.0f, 0.22f);
    drawFilledCircle(0, 0, 24);
    drawFilledCircle(28, 12, 28);
    drawFilledCircle(58, 0, 22);
    drawFilledCircle(26, -10, 18);
}

void drawCloud(float x, float y) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(1.4f, 1.4f, 1.0f);
    drawCloudUnit();
    glPopMatrix();
}

void drawBird(float x, float y, float s, float flap) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(s, s, 1.0f);

    glColor3f(0.05f, 0.05f, 0.05f);

    glBegin(GL_POLYGON);
    glVertex2f(-8, 0);
    glVertex2f(-2, 4);
    glVertex2f(6, 3);
    glVertex2f(10, 0);
    glVertex2f(6, -3);
    glVertex2f(-2, -4);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(10, 0);
    glVertex2f(14, 1.5f);
    glVertex2f(14, -1.5f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-8, 0);
    glVertex2f(-14, 4);
    glVertex2f(-11, 0);
    glVertex2f(-8, 0);
    glVertex2f(-14, -4);
    glVertex2f(-11, 0);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-2, 1);
    glVertex2f(-22, 9 + flap);
    glVertex2f(-5, -1);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(2, 1);
    glVertex2f(22, 9 + flap);
    glVertex2f(5, -1);
    glEnd();

    glPopMatrix();
}

void drawBirds() {
    float flap1 = 4.0f * sin(birdMove * 0.14f);
    float flap2 = 3.4f * sin(birdMove * 0.12f + 0.8f);    //.....sin diye flap value ber kore
    float flap3 = 4.1f * sin(birdMove * 0.15f + 1.4f);

    drawBird(640 + birdMove, 600, 0.9f, flap1);
    drawBird(710 + birdMove, 625, 0.75f, flap2);
    drawBird(785 + birdMove, 605, 1.0f, flap3);
}

void drawMountains() {
    glColor3f(0.42f, 0.28f, 0.25f);
    glBegin(GL_TRIANGLES);
    glVertex2f(40, 260);   glVertex2f(260, 260);  glVertex2f(150, 430);
    glVertex2f(180, 260);  glVertex2f(460, 260);  glVertex2f(320, 470);
    glVertex2f(380, 260);  glVertex2f(680, 260);  glVertex2f(520, 430);
    glVertex2f(600, 260);  glVertex2f(920, 260);  glVertex2f(760, 455);
    glEnd();

    glColor3f(0.58f, 0.42f, 0.34f);
    glBegin(GL_TRIANGLES);
    glVertex2f(80, 260);   glVertex2f(230, 260);  glVertex2f(155, 365);
    glVertex2f(255, 260);  glVertex2f(415, 260);  glVertex2f(335, 390);
    glVertex2f(455, 260);  glVertex2f(635, 260);  glVertex2f(545, 370);
    glVertex2f(670, 260);  glVertex2f(865, 260);  glVertex2f(770, 385);
    glEnd();
}

void drawLand() {
    glColor3f(0.63f, 0.42f, 0.18f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 210);
    glVertex2f(260, 225);
    glVertex2f(430, 255);
    glVertex2f(560, 245);
    glVertex2f(740, 228);                               //uneven polygon
    glVertex2f(1000, 220);
    glVertex2f(1000, 260);
    glVertex2f(0, 260);
    glEnd();

    glColor3f(0.78f, 0.56f, 0.26f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 180);
    glVertex2f(220, 195);
    glVertex2f(420, 220);
    glVertex2f(610, 210);
    glVertex2f(790, 190);
    glVertex2f(1000, 185);
    glVertex2f(1000, 220);
    glVertex2f(0, 220);
    glEnd();
}

void drawRiver() {
    glColor3f(0.18f, 0.46f, 0.78f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glVertex2f(WIDTH, 180);
    glVertex2f(0, 180);
    glEnd();

    glColor4f(0.82f, 0.92f, 1.0f, 0.18f);
    for (int i = -40; i < WIDTH + 40; i += 70) {
        drawLineDDA(i + waterShift, 34, i + 28 + waterShift, 28);
        drawLineDDA(i + 14 + waterShift, 82, i + 42 + waterShift, 76);
        drawLineDDA(i - 10 + waterShift, 128, i + 22 + waterShift, 122);
    }
}

//water shine kora

void drawReflectionLayer() {
    glColor4f(0.88f, 0.94f, 1.0f, 0.08f);         //alpha>>......shine
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glVertex2f(WIDTH, 180);
    glVertex2f(0, 180);
    glEnd();
}

void drawCottageBody() {
    glColor3f(0.82f, 0.60f, 0.42f);         //body
    glBegin(GL_QUADS);
    glVertex2f(180, 220);
    glVertex2f(410, 220);
    glVertex2f(410, 390);
    glVertex2f(180, 390);
    glEnd();

    glColor3f(0.54f, 0.20f, 0.12f);
    glBegin(GL_TRIANGLES);                    //roof
    glVertex2f(145, 390);
    glVertex2f(445, 390);
    glVertex2f(295, 510);
    glEnd();

    glColor3f(0.35f, 0.16f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(270, 220);                    //door
    glVertex2f(325, 220);
    glVertex2f(325, 330);
    glVertex2f(270, 330);
    glEnd();

    glColor3f(0.90f, 0.95f, 0.98f);                           //window
    glBegin(GL_QUADS);
    glVertex2f(210, 270); glVertex2f(255, 270); glVertex2f(255, 325); glVertex2f(210, 325);
    glVertex2f(335, 270); glVertex2f(380, 270); glVertex2f(380, 325); glVertex2f(335, 325);
    glEnd();

    glColor3f(0.68f, 0.82f, 0.92f);                           //window line
    glBegin(GL_LINES);
    glVertex2f(232.5f, 270); glVertex2f(232.5f, 325);
    glVertex2f(210, 297.5f); glVertex2f(255, 297.5f);
    glVertex2f(357.5f, 270); glVertex2f(357.5f, 325);
    glVertex2f(335, 297.5f); glVertex2f(380, 297.5f);
    glEnd();

    glColor3f(0.96f, 0.82f, 0.22f);                            //knob
    drawFilledCircle(312, 275, 4);

    glColor3f(0.22f, 0.12f, 0.06f);
    drawBresenham(180, 220, 410, 220);
    drawBresenham(410, 220, 410, 390);                         //border
    drawBresenham(410, 390, 180, 390);
    drawBresenham(180, 390, 180, 220);
}

void drawFence() {
    glColor3f(0.48f, 0.24f, 0.10f);
    for (int i = 470; i <= 760; i += 24) drawLineDDA(i, 220, i, 258);
    drawLineDDA(456, 235, 780, 235);
    drawLineDDA(456, 252, 780, 252);
}

void drawTreeTop() {
    glColor3f(0.82f, 0.38f, 0.10f);
    drawFilledCircle(-25, 52, 35);
    drawFilledCircle(20, 60, 42);
    drawFilledCircle(58, 42, 34);
    drawFilledCircle(10, 28, 30);

    glColor3f(0.92f, 0.56f, 0.12f);
    drawFilledCircle(-10, 54, 23);
    drawFilledCircle(34, 50, 24);
    drawFilledCircle(10, 32, 18);

    glColor3f(0.72f, 0.24f, 0.08f);
    drawFilledCircle(52, 70, 16);
    drawFilledCircle(-32, 34, 14);
}

void drawTree(float tx, float ty, float sx, float sy) {
    glPushMatrix();
    glTranslatef(tx, ty, 0.0f);
    glScalef(sx, sy, 1.0f);

    glColor3f(0.40f, 0.20f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(-12, 0);
    glVertex2f(16, 0);
    glVertex2f(16, 132);
    glVertex2f(-12, 132);
    glEnd();

    glBegin(GL_POLYGON);                             // neck connect
    glVertex2f(-20, 118);
    glVertex2f(24, 118);
    glVertex2f(18, 150);
    glVertex2f(-14, 150);
    glEnd();
    // gach matha move 

    glPushMatrix();
    glTranslatef(0.0f, 108.0f, 0.0f);
    glRotatef(treeSway, 0.0f, 0.0f, 1.0f);
    drawTreeTop();
    glPopMatrix();

    glPopMatrix();
}

void drawTreeReflection(float tx, float ty, float sx, float sy) {
    glPushMatrix();
    glTranslatef(tx, 180.0f, 0.0f);
    glScalef(sx, -0.42f * sy, 1.0f);
    glTranslatef(0.0f, -ty, 0.0f);

    glColor4f(0.40f, 0.34f, 0.36f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-12, 0);
    glVertex2f(16, 0);
    glVertex2f(16, 132);
    glVertex2f(-12, 132);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-20, 118);
    glVertex2f(24, 118);
    glVertex2f(18, 150);
    glVertex2f(-14, 150);
    glEnd();

    glColor4f(0.52f, 0.46f, 0.48f, 0.08f);
    glBegin(GL_POLYGON);
    glVertex2f(-60, 160);
    glVertex2f(-30, 190);
    glVertex2f(0, 200);
    glVertex2f(40, 195);
    glVertex2f(75, 170);
    glVertex2f(45, 145);
    glVertex2f(0, 132);
    glVertex2f(-40, 140);
    glEnd();

    glPopMatrix();
}

void drawCottageReflection() {
    glPushMatrix();
    glTranslatef(0.0f, 180.0f, 0.0f);
    glScalef(1.0f, -0.32f, 1.0f);
    glTranslatef(0.0f, -220.0f, 0.0f);

    glColor4f(0.64f, 0.48f, 0.34f, 0.14f);
    glBegin(GL_QUADS);
    glVertex2f(180, 220);
    glVertex2f(410, 220);
    glVertex2f(410, 390);
    glVertex2f(180, 390);
    glEnd();

    glColor4f(0.48f, 0.18f, 0.12f, 0.12f);
    glBegin(GL_TRIANGLES);
    glVertex2f(145, 390);
    glVertex2f(445, 390);
    glVertex2f(295, 510);
    glEnd();

    glColor4f(0.26f, 0.12f, 0.08f, 0.12f);
    glBegin(GL_QUADS);
    glVertex2f(270, 220);
    glVertex2f(325, 220);
    glVertex2f(325, 330);
    glVertex2f(270, 330);
    glEnd();

    glColor4f(0.82f, 0.88f, 0.92f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(210, 270); glVertex2f(255, 270); glVertex2f(255, 325); glVertex2f(210, 325);
    glVertex2f(335, 270); glVertex2f(380, 270); glVertex2f(380, 325); glVertex2f(335, 325);
    glEnd();

    glPopMatrix();
}

void drawBoatBody() {
    glColor3f(0.42f, 0.18f, 0.08f);
    glBegin(GL_POLYGON);
    glVertex2f(-70, 0);
    glVertex2f(70, 0);
    glVertex2f(48, -22);
    glVertex2f(-48, -22);
    glEnd();

    glColor3f(0.63f, 0.30f, 0.12f);
    glBegin(GL_QUADS);
    glVertex2f(-8, 0);
    glVertex2f(8, 0);
    glVertex2f(8, 72);
    glVertex2f(-8, 72);
    glEnd();

    glColor3f(0.95f, 0.90f, 0.76f);
    glBegin(GL_TRIANGLES);
    glVertex2f(8, 68);
    glVertex2f(8, 16);
    glVertex2f(58, 40);
    glEnd();
}

void drawBoat() {
    glPushMatrix();
    glTranslatef(220 + boatX, 120 + 3.2f * sin(boatX * 0.022f), 0.0f);
    drawBoatBody();
    glPopMatrix();
}

void drawLeaf(float x, float y, float angle) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);

    glColor3f(0.88f, 0.44f, 0.08f);
    glBegin(GL_POLYGON);
    glVertex2f(1.8f, 10.0f);
    glVertex2f(8.3f, 2.0f);
    glVertex2f(2.6f, -8.0f);
    glVertex2f(-2.2f, -12.0f);
    glVertex2f(-5.4f, -8.0f);
    glVertex2f(-7.6f, 2.0f);
    glEnd();

    glColor3f(0.58f, 0.24f, 0.06f);
    drawLineDDA(0, 10, -1, -12);

    glPopMatrix();
}

void respawnLeaf(int i) {
    leafTree[i] = rand() % 3;
    leafLanded[i] = false;

    if (leafTree[i] == 0) {
        leafX[i] = 98 + rand() % 45;
        leafY[i] = 360 + rand() % 75;
    }
    else if (leafTree[i] == 1) {
        leafX[i] = 485 + rand() % 45;
        leafY[i] = 365 + rand() % 75;
    }
    else {
        leafX[i] = 600 + rand() % 45;
        leafY[i] = 365 + rand() % 75;
    }

    leafGroundY[i] = 184 + rand() % 8;
    leafSpeed[i] = 0.10f + (rand() % 100) / 700.0f;
    leafDrift[i] = 0.04f + (rand() % 100) / 1200.0f;
    leafAngle[i] = rand() % 360;
}
// ekek pata ekek jayga thke pore

void initLeaves() {
    for (int i = 0; i < LEAF_COUNT; i++) {
        respawnLeaf(i);
        leafY[i] -= rand() % 90;
    }
}

void drawLandedLeaves() {
    for (int i = 0; i < LEAF_COUNT; i++) {
        if (leafLanded[i]) drawLeaf(leafX[i], leafY[i], leafAngle[i]);
    }

    drawLeaf(108, 188, 12);
    drawLeaf(126, 186, -15);

    drawLeaf(490, 187, -8);
    drawLeaf(510, 185, 14);

    drawLeaf(605, 188, 10);
    drawLeaf(623, 186, -14);
}

void drawFallingLeaves() {
    for (int i = 0; i < LEAF_COUNT; i++) {
        if (!leafLanded[i]) drawLeaf(leafX[i], leafY[i], leafAngle[i]);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawSky();
    drawSun();
    drawCloud(100 + cloudMove, 590);
    drawCloud(300 + cloudMove, 620);
    drawCloud(620 + cloudMove, 585);
    drawBirds();

    drawMountains();
    drawLand();
    drawRiver();

    drawCottageReflection();
    drawTreeReflection(120, 220, 1.0f, 1.0f);
    drawTreeReflection(500, 220, 1.0f, 1.0f);
    drawTreeReflection(615, 220, 1.0f, 1.0f);
    drawReflectionLayer();

    drawCottageBody();
    drawFence();

    drawTree(120, 220, 1.0f, 1.0f);
    drawTree(500, 220, 1.0f, 1.0f);
    drawTree(615, 220, 1.0f, 1.0f);

    drawLandedLeaves();
    drawBoat();
    drawFallingLeaves();

    glFlush();
}

void update() {
    boatX += 0.22f;
    if (boatX > 900) boatX = -360.0f;

    waterShift += 0.34f;
    if (waterShift > 70.0f) waterShift = 0.0f;

    cloudMove += 0.04f;
    if (cloudMove > WIDTH + 120) cloudMove = -420.0f;

    birdMove += 0.26f;
    if (birdMove > 420.0f) birdMove = -320.0f;

    treeSway += treeDir;
    if (treeSway > 2.2f || treeSway < -2.2f) treeDir = -treeDir;

    for (int i = 0; i < LEAF_COUNT; i++) {
        if (!leafLanded[i]) {
            leafY[i] -= leafSpeed[i];
            leafX[i] += sin(leafAngle[i] * PI / 180.0f) * leafDrift[i];
            leafAngle[i] += 0.25f;

            if (leafY[i] <= leafGroundY[i]) {
                leafY[i] = leafGroundY[i];
                leafLanded[i] = true;
            }
        }
        else {
            if (rand() % 1800 == 0) respawnLeaf(i);
        }
    }

    glutPostRedisplay();
}

void init() {
    srand((unsigned)time(0));

    glClearColor(0.95f, 0.72f, 0.42f, 1.0f);
    gluOrtho2D(0, WIDTH, 0, HEIGHT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(2.0f);

    initLeaves();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(50, 20);
    glutCreateWindow("Autumn Riverside Cottage");
    init();
    glutDisplayFunc(display);
    glutIdleFunc(update);
    glutMainLoop();
    return 0;
}
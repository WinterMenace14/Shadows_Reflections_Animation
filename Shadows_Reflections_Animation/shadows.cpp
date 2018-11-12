#include "core.h"

//Flag variables
#define V_NUMPOINTS    4
#define U_NUMPOINTS	   4
#define V_NUMKNOTS    (V_NUMPOINTS + 4)
#define U_NUMKNOTS    (U_NUMPOINTS + 4)

// Knot sequences for cubic bezier surface and trims 
GLfloat sknots[V_NUMKNOTS] = { 0., 0., 0., 0., 1., 1., 1., 1. };
GLfloat tknots[U_NUMKNOTS] = { 0., 0., 0., 0., 1., 1., 1., 1. };

// Control points for the flag. The Z values are modified to make it wave
GLfloat ctlpoints[V_NUMPOINTS][U_NUMPOINTS][3] = {
	{ { 0., 30., 0. },{ 10., 30., 0. },{ 20., 30., 0 },{ 30., 30., 0. } }, //30
	{ { 0., 20., 0. },{ 10., 20., 0. },{ 20., 20., 0 },{ 30., 20., 0. } },
	{ { 0., 10., 0. },{ 10., 10., 0. },{ 20., 10., 0 },{ 30., 10., 0. } },
	{ { 0., 0., 0. },{ 10., 0., 0. },{ 20., 0., 0 },{ 30., 0., 0. } }
};

GLUnurbsObj *nurbsflag;

// Shadows
GLfloat light_position[4];
GLfloat shadow_matrix[4][4];
Vec3f floor_normal;
vector<Vec3f> dot_vertex_floor;
float lightAngle = 0.0, lightHeight = 100;
int renderShadow = 1;

// calculate floor normal
void calculate_floor_normal(Vec3f *plane, vector<Vec3f> dot_floor) {
	Vec3<GLfloat> AB = dot_floor[1] - dot_floor[0];
	Vec3<GLfloat> AC = dot_floor[2] - dot_floor[0];
	*plane = AB.cross(AC);
}

// Create a matrix that will project the desired shadow
void shadowMatrix(GLfloat shadowMat[4][4], Vec3f plane_normal, GLfloat lightpos[4]) {
	GLfloat dot;
	Vec3f lightpos_v; lightpos_v.x = lightpos[0]; lightpos_v.y = lightpos[1]; lightpos_v.z = lightpos[2];
	dot = plane_normal.dot(lightpos_v);
	shadowMat[0][0] = dot - lightpos[0] * plane_normal[0];
	shadowMat[1][0] = 0.f - lightpos[0] * plane_normal[1];
	shadowMat[2][0] = 0.f - lightpos[0] * plane_normal[2];
	shadowMat[3][0] = 0.f - lightpos[0] * plane_normal[3];
	shadowMat[0][1] = 0.f - lightpos[1] * plane_normal[0];
	shadowMat[1][1] = dot - lightpos[1] * plane_normal[1];
	shadowMat[2][1] = 0.f - lightpos[1] * plane_normal[2];
	shadowMat[3][1] = 0.f - lightpos[1] * plane_normal[3];
	shadowMat[0][2] = 0.f - lightpos[2] * plane_normal[0];
	shadowMat[1][2] = 0.f - lightpos[2] * plane_normal[1];
	shadowMat[2][2] = dot - lightpos[2] * plane_normal[2];
	shadowMat[3][2] = 0.f - lightpos[2] * plane_normal[3];
	shadowMat[0][3] = 0.f - lightpos[3] * plane_normal[0];
	shadowMat[1][3] = 0.f - lightpos[3] * plane_normal[1];
	shadowMat[2][3] = 0.f - lightpos[3] * plane_normal[2];
	shadowMat[3][3] = dot - lightpos[3] * plane_normal[3];
}

// draw_nurb
void draw_nurb() {
	static GLfloat angle = 0.0;
	int i, j;
	// wave the flag by rotating Z coords though a sine wave
	for (i = 1; i < 4; i++)
		for (j = 0; j < 4; j++)
			ctlpoints[i][j][2] = sin((GLfloat)i + angle);
	angle += 0.1;
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glTranslatef(0, 100.0, 0.0);
	//glScalef(12.5, 12.0, 12.0);
	glRotatef(90, 0., 0., 1.);
	gluBeginSurface(nurbsflag);
	gluNurbsSurface(nurbsflag, V_NUMKNOTS, sknots, U_NUMKNOTS, tknots,
		3 * U_NUMPOINTS, 3,
		&ctlpoints[0][0][0], 4, 4, GL_MAP2_VERTEX_3);
	gluEndSurface(nurbsflag);

	//draw_control_graph(ctlpoints);
	glPopMatrix();
	//glutSwapBuffers();
}

// timer
void timer(int t) {
	draw_nurb();
	glutPostRedisplay();
	glutTimerFunc(1000.0 / 30.0, timer, 0);
}

// init
void init() {

	//material for flag
	GLfloat mat_diffuse[] = { 1.0f, 0.5f, 0.31f, 1. };
	GLfloat mat_specular[] = { 0.5f, 0.5f, 0.5f, 1. };
	GLfloat mat_ambient[] = { 1.0f, 0.5f, 0.31f, 1. };

	//creating materials
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	// general
	ratio = (double)width / (double)height;
	mesh1 = createPlane(2000, 2000, 200);
	mesh2 = createCube();
	mesh3 = createCube();
	mesh4 = createCube();
	calculateNormalPerFace(mesh1);
	calculateNormalPerFace(mesh2);
	calculateNormalPerFace(mesh3);
	calculateNormalPerFace(mesh4);
	calculateNormalPerVertex(mesh1);
	calculateNormalPerVertex(mesh2);
	calculateNormalPerVertex(mesh3);
	calculateNormalPerVertex(mesh4);

	// textures
	loadBMP_custom(textureArray, "./BMP_Files/brick.bmp", 0);
	loadBMP_custom(textureArray, "./BMP_Files/oldbox.bmp", 1);
	display1 = meshToDisplayList(mesh1, 1, 0);
	display2 = meshToDisplayList(mesh2, 2, 1);
	display3 = meshToDisplayList(mesh3, 3, 1);
	display4 = meshToDisplayList(mesh4, 4, 1);

	// floor vertex
	dot_vertex_floor.push_back(Vec3<GLfloat>(-2000.0, 0.0, 2000.0));
	dot_vertex_floor.push_back(Vec3<GLfloat>(2000.0, 0.0, 2000.0));
	dot_vertex_floor.push_back(Vec3<GLfloat>(2000.0, 0.0, -2000.0));
	dot_vertex_floor.push_back(Vec3<GLfloat>(-2000.0, 0.0, -2000.0));
	calculate_floor_normal(&floor_normal, dot_vertex_floor);

	// light
	GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	nurbsflag = gluNewNurbsRenderer();
	gluNurbsProperty(nurbsflag, GLU_SAMPLING_TOLERANCE, 100.0);
	gluNurbsProperty(nurbsflag, GLU_DISPLAY_MODE, GLU_FILL); //GLU_OUTLINE_POLYGON

	// shadow and reflection
	glClearStencil(0);
}

// draw an arrow to visualize the source of light
void drawLightArrow() {

	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 0.0);

	// draw arrowhead. 
	glTranslatef(light_position[0], light_position[1], light_position[2]);
	glRotatef(lightAngle * -180.0 / 3.141592, 0, 1, 0);
	glRotatef(atan(lightHeight / 500) * 180.0 / 3.141592, 0, 0, 1);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, 0);
	glVertex3f(20, 10, 10);
	glVertex3f(20, -10, 10);
	glVertex3f(20, -10, -10);
	glVertex3f(20, 10, -10);
	glVertex3f(20, 10, 10);
	glEnd();

	// draw a  line from light direction
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(50, 0, 0);
	glEnd();
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

// display
void display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// light source position
	light_position[0] = 500 * cos(lightAngle);
	light_position[1] = lightHeight;
	light_position[2] = 500 * sin(lightAngle);
	light_position[3] = 0.0; // directional light
	lightAngle += 0.0005;

	// Calculate Shadow matrix
	shadowMatrix(shadow_matrix, floor_normal, light_position);

	// projection and view
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, width, height);
	gluPerspective(40.0, ratio, 1, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// lookAt 
	gluLookAt(0.0f, 40.0f, 320.0, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f);
	// camera
	glScalef(scale, scale, scale);
	glRotatef(x_angle, 1.0f, 0.0f, 0.0f);
	glRotatef(y_angle, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 0.0f);

	// draw
	glPushMatrix();

	// Tell GL new light source position
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	/************************************************************************/
	//stencil for reflection
	glEnable(GL_STENCIL_TEST); //Start using the stencil
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); //Disable writing colors in frame buffer
	glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF); //Place a 1 where rendered
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE); 	//Replace where rendered
	// PLAIN for the stencil
	glPushMatrix();
	glTranslatef(-900, 0, -900);
	glCallList(display1);
	glPopMatrix();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); //Reenable color
	glEnable(GL_DEPTH_TEST);
	glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Keep the pixel



	// boxes reflections
	glPushMatrix();
	glScalef(1.0, -1.0, 1.0);
	glCallList(display2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(200, 0, 0);
	glScalef(1.0, -1.0, 1.0);
	glCallList(display3);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-200, 0, 0);
	glScalef(1.0, -1.0, 1.0);
	glCallList(display4);
	glPopMatrix();

	// STENCIL-STEP 4. disable it
	glDisable(GL_STENCIL_TEST);
	/************************************************************/

	//create blended plain
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.7, 0.0, 0.0, 0.3);
	glColor4f(1.0, 1.0, 1.0, 0.3);
	glPushMatrix();
	glTranslatef(-900, 0, -900);
	glCallList(display1);
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);

	// Shadows
	if (renderShadow) {
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	}

	// Draw floor using blending to blend in reflection
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 1.0, 0.3);
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glTranslatef(-900, 0, -900);
	glCallList(display1);
	glEnable(GL_LIGHTING);
	glPopMatrix();
	glDisable(GL_BLEND);

	// Shadows
	if (renderShadow) {
		glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
		glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
		//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); 
		//  To eliminate depth buffer artifacts, use glEnable(GL_POLYGON_OFFSET_FILL);
		// Render 50% black shadow color on top of whatever the floor appareance is
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_LIGHTING);  // Force the 50% black.
		glColor4f(0.0, 0.0, 0.0, 0.5);
		glPushMatrix();

		// Project the shadow
		glMultMatrixf((GLfloat *)shadow_matrix);

		// boxes
		glDisable(GL_DEPTH_TEST);
		glCallList(display2);
		glTranslatef(200, 0, 0);
		glCallList(display3);
		glTranslatef(-400, 0, 0);
		glCallList(display4);
		glEnable(GL_DEPTH_TEST);
		glPopMatrix();
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);

		// To eliminate depth buffer artifacts, use glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_STENCIL_TEST);
	}

	// box 1
	glPushMatrix();
	glCallList(display2);
	glPopMatrix();
	// box 2
	glPushMatrix();
	glTranslatef(200, 0, 0);
	glCallList(display3);
	glPopMatrix();
	// box 3
	glPushMatrix();
	glTranslatef(-200, 0, 0);
	glCallList(display4);
	glPopMatrix();

	//drawFlag
	draw_nurb();

	// draw the light arrow	
	drawLightArrow();
	glPopMatrix();
	glutSwapBuffers();
}

// main
void main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	glutCreateWindow("Shadows");
	// callbacks
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(1000.0 / 30.0, timer, 0);
	init();
	glutMainLoop();
}
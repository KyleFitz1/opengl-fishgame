// setup code for compilation in Visual Studio 2017. Copyright Timothy Newman, 2021-2023.
// Re-usable, with or without modification, only for CS 445/545 Fall '23 Code Examples, with all re-use documenting the original source in code comments.
// If modified, nature of modification must be documented. No further use/re-use is authorized without written permission of T. Newman

/***********************************************************
 OpenGL445Setup.h

   This header file contains initialization function calls and set-ups
for basic 3D CS 445/545 Open GL (Mesa) programs that use the GLUT/freeglut.
The initializations involve defining a callback handler (my_reshape_function)
that sets viewing parameters for orthographic 3D display.

   TSN 2022 and 2023 version - for OpenGL 4.3 w/legacy compatibility

 ************************************************************/


 /* reshape callback handler - defines viewing parameters (projection) */

void my_3d_projection(int width, int height)
{
	GLfloat width_bound, height_bound;
	width_bound = (GLfloat)width; height_bound = (GLfloat)height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-width_bound / 2.0, width_bound / 2.0, -height_bound / 2.0, height_bound / 2.0, 400, 2000.0);
	glMatrixMode(GL_MODELVIEW);
}

// upper left corner of canvas, in GLUT (not OpenGL!) coordinates

#define STRT_X_POS 25
#define STRT_Y_POS 25

/* initialization routine */

void my_setup(int width, int height, char *window_name_str)
{
	// Allow for current OpenGL4.3 but backwards compatibility to legacy GL 2.1
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	// To get double buffering, uncomment the following line
	// glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); 
	// below DisplayMode code line does single buffering - if above line is uncommented,
	// this single buffering line will have to be commented out
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(STRT_X_POS, STRT_Y_POS);

	glutCreateWindow(window_name_str);   // Note: this ultimately results in GLUT generating a display event
	glewExperimental = GL_TRUE;
	glewInit();

	glutReshapeFunc(my_3d_projection);
}
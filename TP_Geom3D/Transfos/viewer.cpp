#include "viewer.h"

#include <QKeyEvent>
#include <iomanip>


Viewer::Viewer():
	QGLViewer(),
	ROUGE(1,0,0),
	VERT(0,1,0),
	BLEU(0,0,1),
	JAUNE(1,1,0),
	CYAN(0,1,1),
	MAGENTA(1,0,1),
	BLANC(1,1,1),
	GRIS(0.5,0.5,0.5),
	NOIR(0,0,0),
	m_code(0)
{}


void Viewer::init()
{
	makeCurrent();
	glewExperimental = GL_TRUE;
	int glewErr = glewInit();
	if( glewErr != GLEW_OK )
	{
		qDebug("Error %s", glewGetErrorString(glewErr) ) ;
	}
	std::cout << "GL VERSION = " << glGetString(GL_VERSION) << std::endl;

	// la couleur de fond
	glClearColor(0.2,0.2,0.2,0.0);

	// QGLViewer initialisation de la scene
	setSceneRadius(9.0);
	setSceneCenter(qglviewer::Vec(0.0,0.0,0.0));
	camera()->showEntireScene();

	// initilisation des primitives
	m_prim.gl_init();

	// initialisation variables globales
	m_compteur = 0;
	m_angle1 = 0.0;
	m_angle2 = 0.0;


}


void Viewer::draw_repere(const Mat4& global)
{
    //    // exemple de definition de fonction (lambda) locale
    //    float b=2.2f;
    //    auto fonction_locale = [&] (float a)
    //    {
    //        std::cout << "param a="<< a << " & global b="<< b <<std::endl;
    //    };

    //    //appel
    //    fonction_locale(1.1f);

    Mat4 tr = global;
    auto fleche = [&] (Vec3 coul)
    {
        m_prim.draw_cylinder(tr*translate(0,0,1.5)*scale(0.5f,0.5f,2.0f), coul);
        m_prim.draw_cone(tr*translate(0,0,3), coul);
    };

    fleche(BLEU);
    tr = global*rotateY(90);
    fleche(ROUGE);
    tr = global*rotateX(-90);
    fleche(VERT);

/*
    auto fleche = [&] (Mat4 tr, Vec3 coul)
    {
        m_prim.draw_cylinder(tr*translate(0,0,1.5)*scale(0.5f,0.5f,2.0f), coul);
        m_prim.draw_cone(tr*translate(0,0,3), coul);
    };

    //ORIGINE
    m_prim.draw_sphere(global, BLANC);

    fleche(global, BLEU);
    fleche(global*rotateY(90), ROUGE);
    fleche(global*rotateX(-90), VERT);
*/
    /*
    //Y
    m_prim.draw_cylinder(translate(0,1.5,0)*rotateX(-90)*scale(0.5f,0.5f,2.0f), VERT);
    m_prim.draw_cone(translate(0,3,0)*rotateX(-90), VERT);

    //X
    m_prim.draw_cylinder(translate(1.5,0,0)*rotateY(90)*scale(0.5f,0.5f,2.0f), ROUGE);
    m_prim.draw_cone(translate(3,0,0)*rotateY(90), ROUGE);

    //Z
    m_prim.draw_cylinder(translate(0,0,1.5)*rotateX(0)*scale(0.5f,0.5f,2.0f), BLEU);
    m_prim.draw_cone(translate(0,0,3)*rotateX(0), BLEU);
    */
}


void Viewer::draw_main()
{
    /*
    // Bras
  // Doigt majeur
    m_prim.draw_sphere(Mat4()*scale(1,1,1), BLANC);
    m_prim.draw_cube(translate(2,0,0)*scale(3,1,1),ROUGE);
    m_prim.draw_sphere(Mat4()*translate(4,0,0), BLANC);
    m_prim.draw_cube(translate(5,0,0)*scale(3,1,1),VERT);
    */

    Mat4 transfo;
        // param trf: transfo initiale, a: angle , s: longueur doigt
        auto doigt = [&] (Mat4 trf, float a, float s) -> void
        {
            m_prim.draw_sphere(trf, BLANC);

            trf *= rotateZ(a)*translate(s,0,0);
            m_prim.draw_cube(trf*scale(1.7*s,0.5,0.8), ROUGE);

            trf *= translate(s,0,0);
            m_prim.draw_sphere(trf, BLANC);
            trf *= rotateZ(a) * translate(s,0,0);
            m_prim.draw_cube(trf*scale(1.7*s,0.5,0.8), VERT);

            trf *= translate(s*0.7,0,0);
            m_prim.draw_sphere(trf, BLANC);
            trf *=rotateZ(a*1.2) * translate(s,0,0);
            m_prim.draw_cube(trf*scale(1.2*s,0.5,0.8), BLEU);
        };

        // param trf: transfo initiale, b: angle
        auto paume = [&] (Mat4 trf, float b)
        {
            m_prim.draw_sphere(trf, BLANC);
            trf *= rotateZ(b/5);
            trf *= translate(2,0,0);
            m_prim.draw_cube(trf*scale(3,0.6,3), CYAN);
            trf *= translate(1.5,0,0);

            doigt(trf*translate(-1.0,0,-1.5)*rotateY(70), b*0.6,0.5);
            doigt(trf*translate(0,0,-1)*rotateY(10), b*0.7, 1);
            doigt(trf, b*0.8,1.1);
            doigt(trf*translate(0,0,1)*rotateY(-10), b*0.9,0.9);
        };

        // param trf: transfo initiale, a: angle
        auto bras = [&] (Mat4 trf, float a)
        {
            m_prim.draw_sphere(transfo, BLANC);
            trf *= translate(3,0,0);
            m_prim.draw_cube(trf*scale(5,2,2), CYAN);
            trf *= translate(3,0,0);
            trf *= rotateZ(a/5);
            m_prim.draw_sphere(trf, BLANC);
            trf *= translate(3,0,0);
            m_prim.draw_cube(trf*scale(5.5,1.5,1.5), CYAN);
            trf *= translate(3,0,0);

            paume(trf,m_angle1);
        };

        bras(transfo,m_angle1);
}

void Viewer::draw_basic()
{
       float a = 20.0;

       auto doigt2 = [&] (Mat4 trf) -> void
       {
           m_prim.draw_sphere(trf,BLANC);
           m_prim.draw_cube(trf*rotateZ(a)*translate(1.5,0,0)*scale(2,0.5,0.8),ROUGE);

           trf= trf * rotateZ(a)*translate(3,0,0);

           m_prim.draw_sphere(trf,BLANC);
           m_prim.draw_cube(trf*rotateZ(a)*translate(1.5,0,0)*scale(2,0.5,0.8),VERT);

           trf= trf * rotateZ(a)*translate(3,0,0);

           m_prim.draw_sphere(trf,BLANC);
           m_prim.draw_cube(trf*rotateZ(a)*translate(1.5,0,0)*scale(2,0.5,0.8),BLEU);

       };

       doigt2(Mat4());

       doigt2(translate(0,0,2));

       doigt2(translate(0,0,4));

    /*
	m_prim.draw_sphere(Mat4(), BLANC);
	m_prim.draw_cube(translate(3,0,0), ROUGE);
	m_prim.draw_cone(translate(0,3,0), VERT);
	m_prim.draw_cylinder(translate(0,0,3), BLEU);
    */
}

void Viewer::draw()
{
	makeCurrent();
	m_prim.set_matrices(getCurrentModelViewMatrix(),getCurrentProjectionMatrix());

	Mat4 glob;

	switch(m_code)
	{
		case 0:
            draw_main();
		break;
		case 1:
			draw_repere(glob);
		break;
		case 2:
            draw_repere(glob);
            for(int i = 0 ; i< 360 ; i+=60)
            {
                glob = rotateY(m_compteur+i)*translate(6,0,0)*scale(0.5,0.5,0.5);
                draw_repere(glob);
            }

		break;
        case 3:
            draw_basic();
        break;
        case 4:
			draw_main();
		break;
	}
}


void Viewer::keyPressEvent(QKeyEvent *e)
{

	if (e->modifiers() & Qt::ShiftModifier)
	{
		std::cout << "Shift is on"<< std::endl;
	}

	if (e->modifiers() & Qt::ControlModifier)
	{
		std::cout << "Control is on"<< std::endl;
	}

	switch(e->key())
	{
		case Qt::Key_Escape:
			exit(EXIT_SUCCESS);
			break;

		case Qt::Key_A: // touche 'a'
			if (animationIsStarted())
				stopAnimation();
			else
				startAnimation();
			break;

		case Qt::Key_M:  // change le code execute dans draw()
			m_code = (m_code+1)%4;
			break;
		default:
			break;
	}

	// retrace la fenetre
	updateGL();
	// passe la main a la QGLViewer
	QGLViewer::keyPressEvent(e);
}





void Viewer::animate()
{
    m_compteur += 1;

    int x = m_compteur%180;

    if (x<90)
        m_angle1 = x;
    else
        m_angle1 = 179-x;

    m_angle2 = 0.3*m_angle1;
}



Mat4 Viewer::getCurrentModelViewMatrix() const
{
	GLdouble gl_mvm[16];
	camera()->getModelViewMatrix(gl_mvm);
	Mat4 mvm;
	for(unsigned int i = 0; i < 4; ++i)
	{
		for(unsigned int j = 0; j < 4; ++j)
			mvm[i][j] = float(gl_mvm[i*4+j]);
	}
	return mvm;
}


Mat4 Viewer::getCurrentProjectionMatrix() const
{
	GLdouble gl_pm[16];
	camera()->getProjectionMatrix(gl_pm);
	Mat4 pm;
	for(unsigned int i = 0; i < 4; ++i)
	{
		for(unsigned int j = 0; j < 4; ++j)
			pm[i][j] = float(gl_pm[i*4+j]);
	}
	return pm;
}

#include "viewer.h"
#include <QGLViewer/camera.h>
#include <QGLViewer/vec.h>

#include <QKeyEvent>
#include <iomanip>

Viewer::Viewer():
	QGLViewer(),
	m_render_mode(0),
	ROUGE(1,0,0),
	VERT(0,1,0),
	BLEU(0,0,1),
	JAUNE(1,1,0),
	CYAN(0,1,1),
	MAGENTA(1,0,1),
	BLANC(1,1,1),
	GRIS(0.5,0.5,0.5),
	NOIR(0,0,0),
	m_selected_quad(-1)
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
	setSceneRadius(4.0);
	setSceneCenter(qglviewer::Vec(0.0,0.0,0.0));
	camera()->showEntireScene();
	setSceneRadius(20.0);

	// initialisation variables globales
	m_compteur = 0;

	m_prim.gl_init();

	m_mesh.gl_init();
}



void Viewer::draw_repere(const Mat4& global)
{
    /*
	// affiche un repere de taille 1 place suivant global.
    Mat4 tr = global;
    auto fleche = [&] (Vec3 coul)
    {
        m_prim.draw_cylinder(tr*translate(0,0,1.5)*scale(0.5,0.5,1.95), coul);
        m_prim.draw_cone(tr*translate(0,0,3), coul);
    };

    m_prim.draw_sphere(tr,BLANC);
    fleche(BLEU);

    tr = global*rotateY(90);
    fleche(ROUGE);

    tr = global*rotateX(-90);
    fleche(VERT);
    */
}


void Viewer::draw()
{
	makeCurrent();

	m_mesh.set_matrices(getCurrentModelViewMatrix(),getCurrentProjectionMatrix());
	m_prim.set_matrices(getCurrentModelViewMatrix(),getCurrentProjectionMatrix());

	m_mesh.draw(CYAN);

    draw_repere(m_selected_frame);
}


void Viewer::keyPressEvent(QKeyEvent *event)
{
	switch(event->key())
	{
		case Qt::Key_Escape:
			exit(EXIT_SUCCESS);
			break;

		case Qt::Key_C:
			// Attention ctrl c utilise pour screen-shot !
			if (!(event->modifiers() & Qt::ControlModifier))
				m_mesh.create_cube();
		break;

			// e extrusion
			// +/- decale
			// z/Z shrink
			// t/T tourne

			// Attention au cas m_selected_quad == -1


		default:
			break;
	}

	// retrace la fenetre
	updateGL();
	QGLViewer::keyPressEvent(event);
}


void Viewer::mousePressEvent(QMouseEvent* event)
{
	// recupere le rayon de la souris dans la scene (P,Dir)
	qglviewer::Vec Pq = camera()->unprojectedCoordinatesOf(qglviewer::Vec(event->x(), event->y(), -1.0));
	qglviewer::Vec Qq = camera()->unprojectedCoordinatesOf(qglviewer::Vec(event->x(), event->y(), 1.0));

    Vec3 P(Pq[0],Pq[1],Pq[2]);
	Vec3 Dir(Qq[0]-Pq[0],Qq[1]-Pq[1],Qq[2]-Pq[2]);
/*
    qDebug()<<"P : ("<<P.x<<","<<P.y<<","<<P.z<<")";
    qDebug()<<"Q : ("<<Qq.x<<","<<Qq.y<<","<<Qq.z<<")";
    qDebug()<<"Dir : ("<<Dir.x<<","<<Dir.y<<","<<Dir.z<<")";

*/

    //***
    Vec3 inter(0,0,0);
    if(m_mesh.intersect_ray_quad(P,Dir,0,inter))
        qDebug()<<"Intersection dans ABCD";
    else
       qDebug()<<"Pas d'intersection dans ABCD";

     qDebug()<<"Intersection : ("<<inter.x<<","<<inter.y<<","<<inter.z<<")";
     if(m_mesh.intersect_ray_quad(P,Dir,4,inter))
         qDebug()<<"Intersection dans EFGH";
     else
        qDebug()<<"Pas d'intersection dans EFGH";
      qDebug()<<"Intersection : ("<<inter.x<<","<<inter.y<<","<<inter.z<<")";

      if(m_mesh.intersect_ray_quad(P,Dir,8,inter))
          qDebug()<<"Intersection dans ABFE";
      else
         qDebug()<<"Pas d'intersection dans ABFE";
       qDebug()<<"Intersection : ("<<inter.x<<","<<inter.y<<","<<inter.z<<")";
       m_selected_quad = m_mesh.intersected_visible(P,Dir);
       qDebug()<<"Face trouvée :"<<m_selected_quad;
	if (event->modifiers() & Qt::ShiftModifier)
	{

        //***
		m_selected_quad = m_mesh.intersected_visible(P,Dir);
        qDebug()<<"Face "<<m_selected_quad;
		if (m_selected_quad>=0)
		{
            qDebug()<<"SHIFT APPUYE !";
			m_selected_frame = m_mesh.local_frame(m_selected_quad);
		}
	}

	updateGL();
	QGLViewer::mousePressEvent(event);
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

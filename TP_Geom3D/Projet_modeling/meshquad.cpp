#include "meshquad.h"
#include "matrices.h"

using namespace std;

MeshQuad::MeshQuad():
	m_nb_ind_edges(0)
{

}


void MeshQuad::gl_init()
{
	m_shader_flat = new ShaderProgramFlat();
	m_shader_color = new ShaderProgramColor();

	//VBO
	glGenBuffers(1, &m_vbo);

	//VAO
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glEnableVertexAttribArray(m_shader_flat->idOfVertexAttribute);
	glVertexAttribPointer(m_shader_flat->idOfVertexAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &m_vao2);
	glBindVertexArray(m_vao2);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glEnableVertexAttribArray(m_shader_color->idOfVertexAttribute);
	glVertexAttribPointer(m_shader_color->idOfVertexAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);


	//EBO indices
	glGenBuffers(1, &m_ebo);
	glGenBuffers(1, &m_ebo2);
}

void MeshQuad::gl_update()
{
	//VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * m_points.size() * sizeof(GLfloat), &(m_points[0][0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	std::vector<int> tri_indices;
	convert_quads_to_tris(m_quad_indices,tri_indices);

	//EBO indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,tri_indices.size() * sizeof(int), &(tri_indices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	std::vector<int> edge_indices;
	convert_quads_to_edges(m_quad_indices,edge_indices);
	m_nb_ind_edges = edge_indices.size();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_nb_ind_edges * sizeof(int), &(edge_indices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



void MeshQuad::set_matrices(const Mat4& view, const Mat4& projection)
{
	viewMatrix = view;
	projectionMatrix = projection;
}

void MeshQuad::draw(const Vec3& color)
{

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);

	m_shader_flat->startUseProgram();
	m_shader_flat->sendViewMatrix(viewMatrix);
	m_shader_flat->sendProjectionMatrix(projectionMatrix);
	glUniform3fv(m_shader_flat->idOfColorUniform, 1, glm::value_ptr(color));
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ebo);
	glDrawElements(GL_TRIANGLES, 3*m_quad_indices.size()/2,GL_UNSIGNED_INT,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	m_shader_flat->stopUseProgram();

	glDisable(GL_POLYGON_OFFSET_FILL);

	m_shader_color->startUseProgram();
	m_shader_color->sendViewMatrix(viewMatrix);
	m_shader_color->sendProjectionMatrix(projectionMatrix);
	glUniform3f(m_shader_color->idOfColorUniform, 0.0f,0.0f,0.0f);
	glBindVertexArray(m_vao2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ebo2);
	glDrawElements(GL_LINES, m_nb_ind_edges,GL_UNSIGNED_INT,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	m_shader_color->stopUseProgram();
}
// OK
void MeshQuad::clear()
{
    m_points.clear();
	m_quad_indices.clear();
}

// OK
int MeshQuad::add_vertex(const Vec3& P)
{
    m_points.push_back(P);
    return m_points.size()-1;
}

// OK
void MeshQuad::add_quad(int i1, int i2, int i3, int i4)
{
    m_quad_indices.push_back(i1);

    m_quad_indices.push_back(i2);

    m_quad_indices.push_back(i3);

    m_quad_indices.push_back(i4);

}

// OK
void MeshQuad::convert_quads_to_tris(const std::vector<int>& quads, std::vector<int>& tris)
{
	tris.clear();
	tris.reserve(3*quads.size()/2); // 1 quad = 4 indices -> 2 tris = 6 indices d'ou ce calcul (attention division entiere)

    for(int i=0;i<(int)quads.size();i=i+4)
    {
        tris.push_back(quads.at(i));
        tris.push_back(quads.at(i+1));
        tris.push_back(quads.at(i+2));
        tris.push_back(quads.at(i));
        tris.push_back(quads.at(i+2));
        tris.push_back(quads.at(i+3));
    }
	// Pour chaque quad on genere 2 triangles
	// Attention a respecter l'orientation des triangles
}

// OK
void MeshQuad::convert_quads_to_edges(const std::vector<int>& quads, std::vector<int>& edges)
{
	edges.clear();
	edges.reserve(quads.size()); // ( *2 /2 !)


   for(int i=0;i<(int)quads.size();i=i+4)
    {
        edges.push_back(quads.at(i));
        edges.push_back(quads.at(i+1));

        edges.push_back(quads.at(i+1));
        edges.push_back(quads.at(i+2));

        edges.push_back(quads.at(i+2));
        edges.push_back(quads.at(i+3));

        edges.push_back(quads.at(i+3));
        edges.push_back(quads.at(i));
    }
	// Pour chaque quad on genere 4 aretes, 1 arete = 2 indices.
	// Mais chaque arete est commune a 2 quads voisins !
	// Comment n'avoir qu'une seule fois chaque arete ?
}

// OK
void MeshQuad::create_cube()
{
	clear();
	// ajouter 8 sommets (-1 +1)
    /*
               G ________ F
               /|        /|
             H/_|______E/ |
             |  |      |  |
P(-2,-1,0)   | C|______|__|B
             | /       | /
             |/________|/
             D          A

    Les vecteurs des points sont les vecteurs entre
    le centre du cube (0,0,0) et les coordonées des points.
    */

    // Points de la base
    int a = add_vertex(Vec3 (1,-1,-1));
    int b = add_vertex(Vec3 (1,1,-1));
    int c = add_vertex(Vec3 (-1,1,-1));
    int d = add_vertex(Vec3 (-1,-1,-1));

    // Points opposés à la base
    int e = add_vertex(Vec3 (1,-1,1));
    int f = add_vertex(Vec3 (1,1,1));
    int g = add_vertex(Vec3 (-1,1,1));
    int h = add_vertex(Vec3 (-1,-1,1));

    std::cout<<"Creation du cube"<<std::endl;
	// ajouter 6 faces (sens trigo)
    //base
    add_quad(a,d,c,b);
    // opposée à la base
    add_quad(e,f,g,h);

    // Faces entre face base et face opposée
    add_quad(a,b,f,e);
    add_quad(b,c,g,f);
    add_quad(c,d,h,g);
    add_quad(d,a,e,h);

	gl_update();

    // **** TESTS ****
    /*
    qDebug()<<"Normale à la face ADCB:";
    Vec3 normale = normal_of_quad(m_points.at(a),m_points.at(d),m_points.at(c),m_points.at(b));

    qDebug()<<(normale.x);
    qDebug()<<(normale.y);
    qDebug()<<(normale.z);

    qDebug()<<"Normale à la face DCGH :";
    Vec3 normale2 = normal_of_quad(m_points.at(d),m_points.at(h),m_points.at(g),m_points.at(c));

    qDebug()<<(normale2.x);
    qDebug()<<(normale2.y);
    qDebug()<<(normale2.z);

    qDebug()<<"Aire de la face ADCB :";
    qDebug()<<area_of_quad(m_points.at(a),m_points.at(d),m_points.at(c),m_points.at(b));

    Vec3 p = Vec3(1,-1,-1);
    if(is_points_in_quad(p,m_points.at(a),m_points.at(b),m_points.at(f),m_points.at(e)))
        qDebug()<<"P dans la face";
    else
        qDebug()<<"P n'est pas dans la face";

    is_points_in_quad(Vec3(0,1.5,-1.0),Vec3(-1.0,-1.0,-1.0),Vec3(-1.0,1.0,-1.0),Vec3(1.0,1.0,-1.0),Vec3(1.0,-1.0,-1.0));

    is_points_in_quad(Vec3(1,0,1),m_points.at(e),m_points.at(g),m_points.at(f),m_points.at(a));
    */
}

// OK
Vec3 MeshQuad::normal_of_quad(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D)
{
	// Attention a l'ordre des points !
	// le produit vectoriel n'est pas commutatif U ^ V = - V ^ U
	// ne pas oublier de normaliser le resultat.

    Vec3 BA,BC,DA,DC,CB,CD,AD,AB,victor;
    BA = A-B;
    BC = C-B;
    Vec3 normale_ABC = cross(BC,BA);
    //qDebug()<<"Normale à ABC : ("<<normale_ABC.x<<","<<normale_ABC.y<<","<<normale_ABC.z<<")";
    DC=C-D;
    DA=A-D;
    Vec3 normale_CDA = cross(DA,DC);
    //qDebug()<<"Normale à CDA : ("<<normale_CDA.x<<","<<normale_CDA.y<<","<<normale_CDA.z<<")";


    CB=B-C;
    CD=D-C;
    Vec3 normale_BCD = cross(CD,CB);
    //qDebug()<<"Normale à BCD : ("<<normale_BCD.x<<","<<normale_BCD.y<<","<<normale_BCD.z<<")";

    AD=D-A;
    AB=B-A;
    Vec3 normale_DAB = cross(AB,AD);
    //qDebug()<<"Normale à DAB : ("<<normale_DAB.x<<","<<normale_DAB.y<<","<<normale_DAB.z<<")";

    victor = normale_ABC+normale_CDA+normale_BCD+normale_DAB; // On fait la moyenne des normales
    // On normalise en divisant chaque composante par la norme
    float norme =  sqrt(victor.x*victor.x+victor.y*victor.y+victor.z*victor.z);
    victor.x = victor.x/norme;
    victor.y = victor.y/norme;
    victor.z = victor.z/norme;

    //qDebug()<<"Normale totale à ABCD: ("<<victor.x<<","<<victor.y<<","<<victor.z<<")";
    return victor;
}

// OK
float MeshQuad::area_of_quad(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D)
{
	// aire du quad - aire tri + aire tri

	// aire du tri = 1/2 aire parallelogramme

	// aire parallelogramme: cf produit vectoriel

    /* Calcul de l'aire de ABC et ACD */
    Vec3 BA,BC;
    BA = A-B;
    BC = C-B;
    Vec3 normale_ABC = cross(BC,BA);
    //qDebug()<<"Produit vectoriel BC^BA : ("<<normale_ABC.x<<","<<normale_ABC.y<<","<<normale_ABC.z<<")";
    float norme_ABC =  sqrt(normale_ABC.x*normale_ABC.x+normale_ABC.y*normale_ABC.y+normale_ABC.z*normale_ABC.z);
    //qDebug()<<"Norme de la normale à ABC : "<<norme_ABC;
    float aire_ABC = norme_ABC/2.0f;
    //qDebug()<<"longueur de la normale à ABC : "<<aire_ABC;
    Vec3 DC,DA;
    DC=C-D;
    DA=A-D;
    Vec3 normale_CDA = cross(DA,DC);
    float norme_CDA=  sqrt(normale_CDA.x*normale_CDA.x+normale_CDA.y*normale_CDA.y+normale_CDA.z*normale_CDA.z);
    float aire_CDA = norme_CDA/2.0f;
    qDebug()<<"longueur de la normale à CDA : "<<aire_CDA;

    float aire_ABCD = aire_ABC+aire_CDA;
    qDebug()<<"Aire ABCD : "<<aire_ABCD ;
    return (float)aire_ABCD;
}

float MeshQuad::determinant(Vec3 A,Vec3 B,Vec3 C)
{
     return (float) A.x*((B.y)*(C.z)-(C.y)*(B.z)) - B.x*((A.y*C.z)-(C.y*A.z)) + C.x*((A.y*B.z)-(B.y*A.z));
}

// PAS OK
bool MeshQuad::is_points_in_quad(const Vec3& P, const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D)
{
	// On sait que P est dans le plan du quad.

	// P est-il au dessus des 4 plans contenant chacun la normale au quad et une arete AB/BC/CD/DA ?
	// si oui il est dans le quad

    Vec3 normalOfQuad = normal_of_quad(A,B,C,D);

    auto CalculP = [&] (Vec3 A, Vec3 B, Vec3 P) -> float
    {

        auto getNormal = [&] (Vec3 A, Vec3 B, Vec3 N) -> Vec3
        {

            float px = A.x - B.x;
            float py = A.y - B.y;
            float pz = A.z - B.z;

            float nx = (py*N.z)-(pz*N.y);
            float ny = (pz*N.x)-(px*N.z);
            float nz = (px*N.y)-(py*N.x);

            Vec3 res = Vec3(nx,ny,nz);

            return res;
        };

        Vec3 normal = getNormal(A,B,normalOfQuad);

        float d = (normal.x*A.x+normal.y*A.y+normal.z*A.z)*(-1);

        return normal.x*P.x + normal.y*P.y + normal.z*P.z + d;
    };
/*
    qDebug()<<"Valeur de P / ABN : "<<CalculP(A,B,P);
    qDebug()<<"Valeur de P / BCN : "<<CalculP(B,C,P);
    qDebug()<<"Valeur de P / CDN : "<<CalculP(C,D,P);
    qDebug()<<"Valeur de P / DAN : "<<CalculP(D,A,P);
*/
    if(CalculP(A,B,P)>=0 && CalculP(B,C,P)>=0 && CalculP(C,D,P)>=0 && CalculP(D,A,P)>=0)
        return true;
    else
        return false;
}

// OK
bool MeshQuad::intersect_ray_quad(const Vec3& P, const Vec3& Dir, int q, Vec3& inter)
{
	// recuperation des indices de points
	// recuperation des points
    // calcul de l'equation du plan (N+d)
    // calcul de l'intersection rayon plan
    // I = P + alpha*Dir est dans le plan => calcul de alpha
    // alpha => calcul de I
    // I dans le quad ?
    Vec3 A = m_points.at(m_quad_indices.at(q));
    Vec3 B = m_points.at(m_quad_indices.at(q+1));
    Vec3 C = m_points.at(m_quad_indices.at(q+2));
    Vec3 D = m_points.at(m_quad_indices.at(q+3));


    Vec3 BA,BC,DA,DC,CB,CD,AD,AB;
    BA = A-B;
    BC = C-B;
    Vec3 normale_ABC = cross(BC,BA);
    if(normale_ABC*Dir != Vec3(0,0,0))
    {
        float d = (normale_ABC.x*A.x+normale_ABC.y*A.y+normale_ABC.z*A.z)*(-1);
        float alpha = (-d - normale_ABC.x*P.x - normale_ABC.y*P.y - normale_ABC.z*P.z) / (normale_ABC.x*Dir.x + normale_ABC.y*Dir.y + normale_ABC.z*Dir.z);
        Vec3 interPlan = Vec3(P.x+Dir.x*alpha,P.y+Dir.y*alpha,P.z+Dir.z*alpha);
        if(alpha >0 && interPlan!=Vec3(0,0,0))
        {
            if(is_points_in_quad(interPlan,A,B,C,D))
            {
                inter=interPlan;
                return true;
            }
        }
    }
    DC=C-D;
    DA=A-D;
    Vec3 normale_CDA = cross(DA,DC);
    //qDebug()<<"Normale à CDA : ("<<normale_CDA.x<<","<<normale_CDA.y<<","<<normale_CDA.z<<")";
    if(normale_CDA*Dir != Vec3(0,0,0))
    {
        float d = (normale_CDA.x*A.x+normale_CDA.y*A.y+normale_CDA.z*A.z)*(-1);
        float alpha = (-d - normale_CDA.x*P.x - normale_CDA.y*P.y - normale_CDA.z*P.z) / (normale_CDA.x*Dir.x + normale_CDA.y*Dir.y + normale_CDA.z*Dir.z);
        Vec3 interPlan = Vec3(P.x+Dir.x*alpha,P.y+Dir.y*alpha,P.z+Dir.z*alpha);
        if(alpha >0 && interPlan!=Vec3(0,0,0))
        {
            if(is_points_in_quad(interPlan,A,B,C,D))
            {
                inter=interPlan;
                return true;
            }
        }
    }

    CB=B-C;
    CD=D-C;
    Vec3 normale_BCD = cross(CD,CB);
    //qDebug()<<"Normale à BCD : ("<<normale_BCD.x<<","<<normale_BCD.y<<","<<normale_BCD.z<<")";
    if(normale_BCD*Dir != Vec3(0,0,0))
    {
        float d = (normale_BCD.x*A.x+normale_BCD.y*A.y+normale_BCD.z*A.z)*(-1);
        float alpha = (-d - normale_BCD.x*P.x - normale_BCD.y*P.y - normale_BCD.z*P.z) / (normale_BCD.x*Dir.x + normale_BCD.y*Dir.y + normale_BCD.z*Dir.z);
        Vec3 interPlan = Vec3(P.x+Dir.x*alpha,P.y+Dir.y*alpha,P.z+Dir.z*alpha);
        if(alpha >0 && interPlan!=Vec3(0,0,0))
        {
            if(is_points_in_quad(interPlan,A,B,C,D))
            {
                inter=interPlan;
                return true;
            }
        }
    }

    AD=D-A;
    AB=B-A;
    Vec3 normale_DAB = cross(AB,AD);
    //qDebug()<<"Normale à DAB : ("<<normale_DAB.x<<","<<normale_DAB.y<<","<<normale_DAB.z<<")";
    if(normale_DAB*Dir != Vec3(0,0,0))
    {
        float d = (normale_DAB.x*A.x+normale_DAB.y*A.y+normale_DAB.z*A.z)*(-1);
        float alpha = (-d - normale_DAB.x*P.x - normale_DAB.y*P.y - normale_DAB.z*P.z) / (normale_DAB.x*Dir.x + normale_DAB.y*Dir.y + normale_DAB.z*Dir.z);
        Vec3 interPlan = Vec3(P.x+Dir.x*alpha,P.y+Dir.y*alpha,P.z+Dir.z*alpha);
        if(alpha >0 && interPlan!=Vec3(0,0,0))
        {
            if(is_points_in_quad(interPlan,A,B,C,D))
            {
                inter=interPlan;
                return true;
            }
        }
    }

    return false;
}

// OK
int MeshQuad::intersected_visible(const Vec3& P, const Vec3& Dir)
{
    bool intersectionTrouvee=false;
    int interMin=1000000;
    float DirMin = 10000;
    float distance_intersection= -1;
    // on parcours tous les quads
    // on teste si il y a intersection avec le rayon
    // on garde le plus proche (de P)
    Vec3 inter (0,0,0);
    for(int i=0;i<(int)m_quad_indices.size();i=i+4)
    {
        if(intersect_ray_quad(P,Dir,i,inter)==true)
        {
            if(!intersectionTrouvee)
                intersectionTrouvee=true;
            qDebug()<<"intersection trouvée : ("<<inter.x<<","<<inter.y<<","<<inter.z<<")";
            Vec3 lgInter= P-inter;
            distance_intersection = sqrt(lgInter.x*lgInter.x+lgInter.y*lgInter.y+lgInter.z*lgInter.z);
            if(distance_intersection<DirMin)
            {
                qDebug()<<"On a trouve une intersection plus petite !";
                qDebug()<<"Avant : "<<interMin;
                qDebug()<<"Maintenant : "<<i;
                interMin=i;
                DirMin=distance_intersection;
            }
        }
    }

    if(intersectionTrouvee)
        return interMin;
    else
        return -1;
}

// PAS OK
Mat4 MeshQuad::local_frame(int q)
{

	// Repere locale = Matrice de transfo avec
	// les trois premieres colones: X,Y,Z locaux
	// la derniere colonne l'origine du repere

	// ici Z = N et X = AB
	// Origine le centre de la face
	// longueur des axes : [AB]/2

	// recuperation des indices de points
	// recuperation des points

	// calcul de Z:N puis de X:arete on en deduit Y

	// calcul du centre

	// calcul de la taille

	// calcul de la matrice

    Vec3 A = m_points.at(m_quad_indices.at(q));
    Vec3 B = m_points.at(m_quad_indices.at(q+1));
    Vec3 C = m_points.at(m_quad_indices.at(q+2));
    Vec3 D = m_points.at(m_quad_indices.at(q+3));

    Vec3 diagonale = C-A;
    diagonale.x=diagonale.x/2;
    diagonale.y=diagonale.y/2;
    diagonale.z=diagonale.z/2;
    Vec3 O = A+diagonale;
    //display_vec(O,"O");
    Vec3 X = (B-A); // AB


    Vec3 Y = (A-D); // DA
    //display_vec(X,"X");
    //display_vec(Y,"Y au début (=AD) ");

    Vec3 Z = normal_of_quad(A,B,C,D);
    //display_vec(Z,"Z sans translation");

    float norme_Y = sqrt(Y.x*Y.x+Y.y*Y.y+Y.z*Y.z);
    Y=Y/norme_Y;
    //display_vec(Y,"Y normalisé");
    Y=Y;
    //Y = Vec3 (Y.x/norme_Y,Y.y/norme_Y,Y.z/norme_Y);

    float norme_X = sqrt(X.x*X.x+X.y*X.y+X.z*X.z);
    //X = Vec3 (X.x/norme_X,X.y/norme_X,X.z/norme_X);

    X=X/norme_X;
    //Z = Vec3 (Z.x+O.x,Z.y+O.y,Z.z+O.z);
    float norme_Z = sqrt(Z.x*Z.x+Z.y*Z.y+Z.z*Z.z);
    Z=Z/norme_Z;

    //display_vec(Z,"Z");

    //display_vec(Y,"Y translaté au repère local");
    //display_vec(X,"X translaté au repère local");

    Mat4 transfo;

    transfo[0][0]=X.x;
    transfo[0][1]=X.y;
    transfo[0][2]=X.z;
    transfo[0][3]=0;

    transfo[1][0]=Y.x;
    transfo[1][1]=Y.y;
    transfo[1][2]=Y.z;
    transfo[1][3]=0;

    transfo[2][0]=Z.x;
    transfo[2][1]=Z.y;
    transfo[2][2]=Z.z;
    transfo[2][3]=0;

    transfo[3][0]=O.x;
    transfo[3][1]=O.y;
    transfo[3][2]=O.z;
    transfo[3][3]=1;

    return transfo;
}
// PAS OK
void MeshQuad::extrude_quad(int q)
{
    // recuperation des indices de points

    // recuperation des points

    // calcul de la normale

    // calcul de la hauteur

    // calcul et ajout des 4 nouveaux points

    // on remplace le quad initial par le quad du dessu

    // on ajoute les 4 quads des cotes

    Vec3 A = m_points.at(m_quad_indices.at(q));
    Vec3 B = m_points.at(m_quad_indices.at(q+1));
    Vec3 C = m_points.at(m_quad_indices.at(q+2));
    Vec3 D = m_points.at(m_quad_indices.at(q+3));

    Vec3 Z = normal_of_quad(A,B,C,D);
    float norme_Z = sqrt(Z.x*Z.x+Z.y*Z.y+Z.z*Z.z);
    Z=Z/norme_Z;

    float hauteur = sqrt(area_of_quad(A,B,C,D));
    Z=Z*hauteur;

    Vec3 A_extrude = A+Z;
    Vec3 B_extrude = B+Z;
    Vec3 C_extrude = C+Z;
    Vec3 D_extrude = D+Z;

    int i = add_vertex(A_extrude);
    int j = add_vertex(B_extrude);
    int k = add_vertex(C_extrude);
    int l = add_vertex(D_extrude);

    add_quad(m_quad_indices.at(q),m_quad_indices.at(q+1),j,i);
    add_quad(m_quad_indices.at(q+1),m_quad_indices.at(q+2),k,j);
    add_quad(m_quad_indices.at(q+2),m_quad_indices.at(q+3),l,k);
    add_quad(m_quad_indices.at(q+3),m_quad_indices.at(q),i,l);
    add_quad(i,j,k,l);
    gl_update();
}


void MeshQuad::decale_quad(int q, float d)
{
	// recuperation des indices de points

	// recuperation des (references de) points

	// calcul de la normale

	// modification des points

    Vec3 A = m_points.at(m_quad_indices.at(q));
    Vec3 B = m_points.at(m_quad_indices.at(q+1));
    Vec3 C = m_points.at(m_quad_indices.at(q+2));
    Vec3 D = m_points.at(m_quad_indices.at(q+3));

    Vec3 Z = normal_of_quad(A,B,C,D);
    float norme_Z = sqrt(Z.x*Z.x+Z.y*Z.y+Z.z*Z.z);
    Z=Z/norme_Z;

    float hauteur = sqrt(area_of_quad(A,B,C,D))*d;
    Z=Z*hauteur;

    A = A+Z;
    B = B+Z;
    C = C+Z;
    D = D+Z;
    /*
    int a_decale = add_vertex(A);
    int b_decale = add_vertex(B);
    int c_decale = add_vertex(C);
    int d_decale = add_vertex(D);
    */
    m_points.at(m_quad_indices.at(q))=A;
    m_points.at(m_quad_indices.at(q+1))=B;
    m_points.at(m_quad_indices.at(q+2))=C;
    m_points.at(m_quad_indices.at(q+3))=D;
/*
    m_quad_indices.at(q)=a_decale;
    m_quad_indices.at(q+1)=b_decale;
    m_quad_indices.at(q+2)=c_decale;
    m_quad_indices.at(q+3)=d_decale;
    */

	gl_update();
}


void MeshQuad::shrink_quad(int q, float s)
{
	// recuperation des indices de points

	// recuperation des (references de) points

	// ici  pas besoin de passer par une matrice
	// calcul du centre

	 // modification des points

    Vec3 A = m_points.at(m_quad_indices.at(q));
    Vec3 D = m_points.at(m_quad_indices.at(q+1));
    Vec3 C = m_points.at(m_quad_indices.at(q+2));
    Vec3 B = m_points.at(m_quad_indices.at(q+3));

    Vec3 CentreA = (A-C);
    Vec3 CentreB = (B-D);
    Vec3 CentreC = (C-A);
    Vec3 CentreD = (D-B);
    CentreA.x=CentreA.x/2;
    CentreA.y=CentreA.y/2;
    CentreA.z=CentreA.z/2;
    CentreB.x=CentreB.x/2;
    CentreB.y=CentreB.y/2;
    CentreB.z=CentreB.z/2;
    CentreC.x=CentreC.x/2;
    CentreC.y=CentreC.y/2;
    CentreC.z=CentreC.z/2;
    CentreD.x=CentreD.x/2;
    CentreD.y=CentreD.y/2;
    CentreD.z=CentreD.z/2;
    A = A+(CentreA)*(s-1);
    B = B+(CentreB)*(s-1);
    C = C+(CentreC)*(s-1);
    D = D+(CentreD)*(s-1);
    m_points.at(m_quad_indices.at(q))=A;
    m_points.at(m_quad_indices.at(q+1))=D;
    m_points.at(m_quad_indices.at(q+2))=C;
    m_points.at(m_quad_indices.at(q+3))=B;

	gl_update();
}

// PAS OK
void MeshQuad::tourne_quad(int q, float a)
{
	// recuperation des indices de points

	// recuperation des (references de) points

	// generation de la matrice de transfo:
	// tourne autour du Z de la local frame
	// indice utilisation de glm::inverse()

	// Application au 4 points du quad

    Vec3 A = m_points.at(m_quad_indices.at(q));
    Vec3 D = m_points.at(m_quad_indices.at(q+1));
    Vec3 C = m_points.at(m_quad_indices.at(q+2));
    Vec3 B = m_points.at(m_quad_indices.at(q+3));

    Vec3 Z = normal_of_quad(A,B,C,D);
/*
    Mat4 transfo;
    if (a>=0)
    {
        transfo = rotateZ(a);
    }
    if(a<0)
    {
        a*=-1;
        transfo = glm::inverse(rotateZ(a));
    }
    */
    glm::mat3 transfo;
    glm::mat3 transfo1;
    glm::mat3 transfo2;
    glm::mat3 transfo3;

    transfo1[0][0] = Z.x*Z.x;
    transfo1[0][1] = Z.y*Z.x;
    transfo1[0][2] = Z.z*Z.x;

    transfo1[1][0] = Z.x*Z.y;
    transfo1[1][1] = Z.y*Z.y;
    transfo1[1][2] = Z.z*Z.y;

    transfo1[2][0] = Z.x*Z.z;
    transfo1[2][2] = Z.y*Z.z;
    transfo1[2][2] = Z.z*Z.z;

    //----------------------
    transfo2[0][0] = 1;
    transfo2[0][1] = 0;
    transfo2[0][2] = 0;

    transfo2[1][0] = 0;
    transfo2[1][1] = 1;
    transfo2[1][2] = 0;

    transfo2[2][0] = 0;
    transfo2[2][2] = 0;
    transfo2[2][2] = 1;
    //----------------------
    transfo3[0][0] = 0;
    transfo3[0][1] = Z.z;
    transfo3[0][2] = -Z.y;

    transfo3[1][0] = -Z.z;
    transfo3[1][1] = 0;
    transfo3[1][2] = Z.x;

    transfo3[2][0] = Z.y;
    transfo3[2][2] = -Z.x;
    transfo3[2][2] = 0;

    transfo = (1-cos(a))*transfo1+cos(a)*transfo2+sin(a)*transfo3;

/*
    transfo[0][0]=cos(a);
    transfo[0][1]=sin(a);
    transfo[0][2]=0;
    transfo[0][3]=0;

    transfo[1][0]=-sin(a);
    transfo[1][1]=cos(a);
    transfo[1][2]=0;
    transfo[1][3]=0;

    transfo[2][0]=0;
    transfo[2][1]=0;
    transfo[2][2]=0;
    transfo[2][3]=0;

    transfo[3][0]=0;
    transfo[3][1]=0;
    transfo[3][2]=0;
    transfo[3][3]=1;
*/
/*
    Vec4 A4 (A.x,A.y,A.z,1.0);
    A4 = transfo*A4;
    A = Vec3 (A4.x/A4[3],A4.y/A4[3],A4.z/A4[3]);
    Vec4 B4 (B.x,B.y,B.z,1.0);
    B4 = transfo*B4;
    B = Vec3 (B4.x/B4[3],B4.y/B4[3],B4.z/B4[3]);
    Vec4 C4 (C.x,C.y,C.z,1.0);
    C4 = transfo*C4;
    C = Vec3 (C4.x/C4[3],C4.y/C4[3],C4.z/C4[3]);
    Vec4 D4 (D.x,D.y,D.z,1.0);
    D4 = transfo*D4;
    D = Vec3 (D4.x/D4[3],D4.y/D4[3],D4.z/D4[3]);
*/
    A=A*transfo;
    B=B*transfo;
    C=C*transfo;
    D=D*transfo;
    m_points.at(m_quad_indices.at(q))=A;
    m_points.at(m_quad_indices.at(q+1))=D;
    m_points.at(m_quad_indices.at(q+2))=C;
    m_points.at(m_quad_indices.at(q+3))=B;

	gl_update();
}

void MeshQuad::display_vec(Vec3 X,QString name){

    qDebug()<<name<<" : ("<<X.x<<","<<X.y<<","<<X.z<<")";
}

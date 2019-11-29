#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "shaderLoader.h" //narzŕdzie do │adowania i kompilowania shaderˇw z pliku

#include <fstream>

//funkcje algebry liniowej
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective


//Wymiary okna
int screen_width = 1280;
int screen_height = 720;


int pozycjaMyszyX; // na ekranie
int pozycjaMyszyY;
int mbutton; // wcisiety klawisz myszy

double kameraX = 0.0;
double kameraY = 0.0;
double kameraZ = 0.0;
double kameraD = -3.0;
double kameraPredkosc;
double kameraKat = 20;
double kameraPredkoscObrotu;
double poprzednie_kameraX;
double poprzednie_kameraY;
double poprzednie_kameraZ;
double poprzednie_kameraD;

double rotation = 0;


//macierze
glm::mat4 MV; //modelview - macierz modelu i świata
glm::mat4 P;  //projection - macierz projekcji, czyli naszej perspektywy


float vertices[] = {
        //dół
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, -0.5f,
        //góra
        -0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        //lewo
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        //prawo
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        //przód
        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        //tył
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
};
int faceVerticesSize;
float *faceVertices;

void modifyVerticies()
{
    for(int i=0;i<sizeof(vertices)/sizeof(float);i++)
        vertices[i]+=.5;
}

void importVerticies()
{
    modifyVerticies();
    int il;
    int i=0;
    std::fstream plik;
    plik.open( "dane3d.dat", std::ios::in | std::ios::out );
    if(!plik.good()||plik.eof())
        fprintf(stderr,"plik nie dziala");
    plik>>il;
    il *=3;
    faceVerticesSize = il;

    faceVertices= new float[il];
    while(i<il && !plik.eof())
        plik>>faceVertices[i++];
    plik.close();
    //for(i=0;i<20;)
        //printf("%f %f %f\n",faceVertices[i++],faceVertices[i++],faceVertices[i++]);
}

//shaders
GLuint programID = 0;
GLuint programID2 = 0;

unsigned int VBO;
unsigned int VBO2;

/*###############################################################*/
void mysz(int button, int state, int x, int y) {
    mbutton = button;
    switch (state) {
        case GLUT_UP:
            break;
        case GLUT_DOWN:
            pozycjaMyszyX = x;
            pozycjaMyszyY = y;
            poprzednie_kameraY = kameraY;
            poprzednie_kameraZ = kameraZ;
            poprzednie_kameraD = kameraD;
            break;

    }
}

/*******************************************/
void mysz_ruch(int x, int y) {
    if (mbutton == GLUT_LEFT_BUTTON) {
        kameraY = poprzednie_kameraY - (pozycjaMyszyX - x) * 0.1;
        kameraZ = poprzednie_kameraZ - (pozycjaMyszyY - y) * 0.1;
    }
    if (mbutton == GLUT_RIGHT_BUTTON) {
        kameraD = poprzednie_kameraD + (pozycjaMyszyY - y) * 0.1;
    }

}

/******************************************/

double dx = 0;

void klawisz(GLubyte key, int x, int y) {
    switch (key) {

        case 27:    /* Esc - koniec */
            exit(1);
            break;

        case 'x':
            dx += 1;
            break;
        case '1':
            kameraX +=1;
            break;
        case '2':
            kameraX -=1;
            break;
    }


}

/*###############################################################*/
void rysuj(void) {

    //GLfloat color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    //glClearBufferfv(GL_COLOR, 0, color);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Kasowanie ekranu

    glUseProgram(programID); //u┐yj programu, czyli naszego shadera

    MV = glm::mat4(1.0f);  //macierz jednostkowa
    MV = glm::translate(MV, glm::vec3(dx, 0, kameraD));
    MV = glm::rotate(MV, (float) glm::radians(kameraZ), glm::vec3(1, 0, 0));
    MV = glm::rotate(MV, (float) glm::radians(kameraX), glm::vec3(0, 1, 0));
    MV = glm::rotate(MV, (float) glm::radians(kameraY), glm::vec3(0, 0, 1));

    glm::mat4 MVP = P * MV;

    /*Zmienne   jednorodne   (ang. uniform variable), zwane  także  zmiennym  globalnymi,sązmiennymi,
        których  wartośc  jest  stała  w  obrębie  obiektu programu. Shadery  mogą zmienne jednorodne tylko  odczytywac */
    GLuint MVP_id = glGetUniformLocation(programID, "MVP"); // pobierz lokalizację zmiennej 'uniform' "MV" w programie
    glUniformMatrix4fv(MVP_id, 1, GL_FALSE,
                       &(MVP[0][0]));       // wyślij tablicę mv do lokalizacji "MV", która jest typu mat4
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0); // pierwszy buform atrybuˇw: wierzcho│ki
    glVertexAttribPointer(
            0,                  // atrybut 0. musi odpowiadaŠ atrybutowi w programie shader
            3,                  // wielkoťŠ (x,y,z)
            GL_FLOAT,           // typ
            GL_FALSE,           // czy znormalizowany [0-1]?
            0,                  // stride
            (void *) 0            // array buffer offset
    );
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_QUADS, 0, sizeof(vertices)/ sizeof(float)); // Zaczynamy od 0 i rysujemy wszystkie wierzcho│ki


    glUseProgram(programID2);
    MVP_id = glGetUniformLocation(programID2, "MVP"); // pobierz lokalizację zmiennej 'uniform' "MV" w programie
    glUniformMatrix4fv(MVP_id, 1, GL_FALSE,
                       &(MVP[0][0]));       // wyślij tablicę mv do lokalizacji "MV", która jest typu mat4
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glEnableVertexAttribArray(0); // pierwszy buform atrybuˇw: wierzcho│ki
    glVertexAttribPointer(
            0,                  // atrybut 0. musi odpowiadaŠ atrybutowi w programie shader
            3,                  // wielkoťŠ (x,y,z)
            GL_FLOAT,           // typ
            GL_FALSE,           // czy znormalizowany [0-1]?
            0,                  // stride
            (void *) 0            // array buffer offset
    );
    glDrawArrays(GL_POINTS, 0, faceVerticesSize);

    glFlush();
    glutSwapBuffers();
}

/*###############################################################*/
void rozmiar(int width, int height) {
    screen_width = width;
    screen_height = height;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, screen_width, screen_height);

    P = glm::perspective(glm::radians(60.0f), (GLfloat) screen_width / (GLfloat) screen_height, 1.0f, 1000.0f);

    glutPostRedisplay(); // Przerysowanie sceny
}

/*###############################################################*/
void idle() {

    glutPostRedisplay();
}

/*###############################################################*/
GLfloat k = 0.05;
GLfloat ad = 0.0;

void timer(int value) {

    ad += k;

    if (ad > 1 || ad < 0)
        k = -k;

    //GLfloat attrib[] = { ad, 0.0f,0.0f };
    // Aktualizacja wartości atrybutu wejściowego 1.
    //glVertexAttrib3fv(1, attrib);

    /*

    W vertex_shader np:
    layout (location = 1) in vec3 incolor;

    */
    glutTimerFunc(20, timer, 0);
}

/*###############################################################*/
int main(int argc, char **argv) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screen_width, screen_height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Przyklad 3");

    glewInit(); //init rozszerzeszeń OpenGL z biblioteki GLEW

    glutDisplayFunc(rysuj);            // def. funkcji rysuj¦cej
    glutIdleFunc(idle);            // def. funkcji rysuj¦cej w czasie wolnym procesoora (w efekcie: ci¦gle wykonywanej)
    glutTimerFunc(20, timer, 0);
    glutReshapeFunc(rozmiar); // def. obs-ugi zdarzenia resize (GLUT)

    glutKeyboardFunc(klawisz);        // def. obsługi klawiatury
    glutMouseFunc(mysz);        // def. obsługi zdarzenia przycisku myszy (GLUT)
    glutMotionFunc(mysz_ruch); // def. obsługi zdarzenia ruchu myszy (GLUT)


    glEnable(GL_DEPTH_TEST);

    //glPointSize(3.0f);
    importVerticies();
    //tworzenie bufora wierzcholków
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //vertices=faceVertices;
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, faceVerticesSize*sizeof(float), faceVertices, GL_STATIC_DRAW);

    /* úadowanie shadera, tworzenie programu *************************/
    /* i linkowanie go oraz sprawdzanie b│ŕdˇw! **********************/
    programID = loadShaders("vertex_shader.glsl", "fragment_shader.glsl");
    programID2 = loadShaders("vertex_shader.glsl", "fragment_shader2.glsl");

    glutMainLoop();                    // start

    delete faceVertices;
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VBO2);

    return (0);
}


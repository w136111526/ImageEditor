#include <iostream>
#include <map>
#include <fstream>
#include <cmath>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "Image.h"

using namespace std;

struct pixel{
        unsigned char r,g,b;
        pixel():r(0),g(0),b(0){}
        pixel(unsigned char _r, unsigned char _g, unsigned char _b):r(_r),g(_g),b(_b){}
        pixel(const pixel& p){
            r = p.r;
            g = p.g;
            b = p.b;
        }
        void set(unsigned char _r, unsigned char _g, unsigned char _b){
            r = _r;
            g = _g;
            b = _b;
        }
        void set(pixel p){
            r = p.r;
            g = p.g;
            b = p.b;
        }
        bool is(unsigned char _r, unsigned char _g, unsigned char _b){
            return r==_r && g==_g && b==_b;
        }
        bool operator==(const pixel& p){
            return r==p.r && g==p.g && b==p.b;
        }
        bool operator!=(const pixel& p){
            return !(*this==p);
        }
    };

class pbm : public sf::Drawable{
    int _tipo;
    pixel **_imagen;
    int _x, _y;

    void fill(pixel**& arr, int x, int y){
        if(x==0 || y==0){
            arr=0;
            return;
        }
        arr = new pixel*[x];
        for(int i=0; i<x; i++)
            arr[i] = new pixel[y];
    }
    void destroy(pixel** arr, int x, int y){
        if(arr==0) return;
        if(y>0){
            for(int i=0; i<x; i++)
                if(y==1) delete arr[i];
                else delete[] arr[i];
        }
        if(x==1) delete arr;
        else if(x>1) delete[] arr;
    }

public:
    pbm():_tipo(0),_imagen(0),_x(0),_y(0){}
    pbm(int x, int y, pixel p = pixel(0,0,0)):_tipo(0),_imagen(0),_x(0),_y(0){
        crear(x,y,p);
    }
    ~pbm(){
        destroy(_imagen, _x, _y);
    }
    int tipo(){return _tipo;}
    int getX()const{return _x;}
    int getY()const{return _y;}
    bool valido(){
        return _tipo>=1 && _tipo<=6 && _x>0 && _y>0;
    }
    pixel getPixel(int x, int y){
        if(x<0 || x>=_x || y<0 || y>=_y) return pixel();
        return _imagen[x][y];
    }
    void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b){
        if(x<0 || x>=_x || y<0 || y>=_y) return;
        _imagen[x][y].set(r,g,b);
    }
    void crear(int x, int y, pixel p = pixel(0,0,0)){
        _tipo = 6;
        if(x<1||y<1) return;
        if(_x>0 && _y>0)
            destroy(_imagen, _x, _y);
        fill(_imagen, x, y);
        for(int i=0; i<x; i++)
            for(int j=0; j<y; j++)
                _imagen[i][j] = p;
        _x=x;
        _y=y;
    }
    bool leer(string archivo){
        int x=0, y=0;
        int tipo;
        ifstream f(archivo,ios::binary);
        if(!f) return false;
        string t;
        getline(f,t,(char)0x0A);
        if(t.size()!=2 || (t[0]!='P' && t[0]!='p') || t[1]<'1' || t[1]>'6')
            return false;
        tipo = t[1]-'0';
        getline(f,t,(char)0x0A);
        x = atoi(t.c_str());
        getline(f,t,(char)0x0A);
        y = atoi(t.c_str());
        getline(f,t,(char)0x0A);//profundidad
        if(y==0 || x==0)
            return false;
        fill(_imagen,x,y);
        for(int j=0;j<y;j++)
            for(int i=0;i<x;i++){
                f.read((char*)&_imagen[i][j].r, 1);
                f.read((char*)&_imagen[i][j].g, 1);
                f.read((char*)&_imagen[i][j].b, 1);
            }
        _x=x;
        _y=y;
        _tipo = tipo;
        return true;
    }
    bool exportar(string archivo){
        if(!valido()) return false;
        ofstream f(archivo,ios::trunc|ios::binary);
        if(!f) return false;
        char salto = 0x0A;
        f << "P" << _tipo;
        f.write(&salto,1);
        f << _x;
        f.write(&salto,1);
        f << _y;
        f.write(&salto,1);
        f << "255";
        f.write(&salto,1);
        for(int j=0;j<_y;j++)
            for(int i=0;i<_x;i++){
                f.write((char*)&_imagen[i][j].r,1);
                f.write((char*)&_imagen[i][j].g,1);
                f.write((char*)&_imagen[i][j].b,1);
            }
        return true;
    }
    void carboncillo(unsigned char tolerancia=255){
        pixel **t;
        fill(t, _x, _y);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                for(int n=0; n<9; n++){
                    if(!(n==4 || i+n%3-1<0 || i+n%3-1>=_x || j+n/3-1<0 || j+n/3-1>=_y))
                        if((int)_imagen[i+n%3-1][j+n/3-1].r-(int)_imagen[i][j].r>tolerancia ||
                           (int)_imagen[i+n%3-1][j+n/3-1].g-(int)_imagen[i][j].g>tolerancia ||
                           (int)_imagen[i+n%3-1][j+n/3-1].b-(int)_imagen[i][j].b>tolerancia)
                            break;
                    if(n==8){
                        t[i][j].set(255,255,255);
                    }
                }
        destroy(_imagen,_x,_y);
        _imagen = t;
    }
    void escalaDeGrises(){
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                int t = _imagen[i][j].r+_imagen[i][j].g+_imagen[i][j].b;
                t/=3;
                _imagen[i][j].set(t,t,t);
            }
    }
    void blancoYNegro(){
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                double dt = _imagen[i][j].r+_imagen[i][j].g+_imagen[i][j].b;
                int t = dt/3>=127.5?255:0;
                _imagen[i][j].set(t,t,t);

            }
    }
    void posterizar(unsigned char niveles){
        if(niveles<2) niveles = 2;
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                double valor = double(_imagen[i][j].r)/255.0;
                valor*=niveles-1;
                valor = round(valor);
                valor/=niveles-1;
                _imagen[i][j].r = valor*255;
                valor = double(_imagen[i][j].g)/255.0;
                valor*=niveles-1;
                valor = round(valor);
                valor/=niveles-1;
                _imagen[i][j].g = valor*255;
                valor = double(_imagen[i][j].b)/255.0;
                valor*=niveles-1;
                valor = round(valor);
                valor/=niveles-1;
                _imagen[i][j].b = valor*255;
            }
    }
    void invertir(){
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                _imagen[i][j].r = 255-_imagen[i][j].r;
                _imagen[i][j].g = 255-_imagen[i][j].g;
                _imagen[i][j].b = 255-_imagen[i][j].b;
            }
    }
    void desenfocar(const int size){
        pixel **t;
        fill(t,_x,_y);
        int s = (1+size*2);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                int r,g,b,c;
                r=g=b=c=0;
                for(int n=0; n<=s*s; n++){
                    if(i+n%s-size>=0 && i+n%s-size<_x && j+n/s-size>=0 && j+n/s-size<_y){
                        ++c;
                        r+=_imagen[i+n%s-size][j+n/s-size].r;
                        g+=_imagen[i+n%s-size][j+n/s-size].g;
                        b+=_imagen[i+n%s-size][j+n/s-size].b;
                    }
                }
                if(c!=0)
                    t[i][j].set(r/c,g/c,b/c);
            }

        destroy(_imagen,_x,_y);
        _imagen = t;
    }
    void eliminarColores(bool r, bool g, bool b){
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                if(r) _imagen[i][j].r=0;
                if(g) _imagen[i][j].g=0;
                if(b) _imagen[i][j].b=0;
            }
    }
    void maximizarColores(bool r, bool g, bool b){
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                if(r) _imagen[i][j].r=255;
                if(g) _imagen[i][j].g=255;
                if(b) _imagen[i][j].b=255;
            }
    }
    void craze(){
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                bool r,g,b,r2,g2,b2;
                r=g=b=r2=g2=b2=false;
                if(_imagen[i][j].r>127) r2=true;
                else if(_imagen[i][j].r<128) r=true;
                if(_imagen[i][j].g>127) g2=true;
                else if(_imagen[i][j].g<128) g=true;
                if(_imagen[i][j].b>127) b2=true;
                else if(_imagen[i][j].b<128) b=true;
                if(r){
                    _imagen[i][j].g/=2;
                    _imagen[i][j].b/=2;
                }
                if(g){
                    _imagen[i][j].r/=2;
                    _imagen[i][j].b/=2;
                }
                if(b){
                    _imagen[i][j].g/=2;
                    _imagen[i][j].r/=2;
                }
                if(r2){
                    _imagen[i][j].r+=(255-_imagen[i][j].r)/2;
                }
                if(g2){
                    _imagen[i][j].g+=(255-_imagen[i][j].g)/2;
                }
                if(b2){
                    _imagen[i][j].b+=(255-_imagen[i][j].b)/2;
                }
            }
    }
    void reemplazarColor(pixel buscar, unsigned char tolerancia, pixel fin){
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                if(abs((int)_imagen[i][j].r+(int)_imagen[i][j].g+(int)_imagen[i][j].b
                    -(int)buscar.r-(int)buscar.g-(int)buscar.b)<=tolerancia)
                    _imagen[i][j] = fin;
            }
    }
    void bloom(int range){
        pixel** t;
        fill(t,_x,_y);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                int r=0, g=0, b=0;
                int rx=0, ry=0,
				    gx=0, gy=0,
					bx=0, by=0;
                for(int n=(i-range<0?0:-range); n<=range && i+n<_x; n++)
                    for(int m=(j-range<0?0:-range); m<=range && j+m<_y && n!=m; m++){
                        if(_imagen[i+n][j+m].r>r){
                            r = _imagen[i+n][j+m].r;
                            rx = n;
							ry = m;
                        }
                        if(_imagen[i+n][j+m].g>g){
                            g = _imagen[i+n][j+m].g;
                            gx = n;
							gy = m;
                        }
                        if(_imagen[i+n][j+m].b>b){
                            b = _imagen[i+n][j+m].b;
                            bx = n;
							by = m;
                        }
                    }
				float rd = sqrt(rx*rx+ry*ry),
					  gd = sqrt(gx*gx+gy*gy),
					  bd = sqrt(bx*bx+by*by);
                t[i][j] = _imagen[i][j];
                if(r-(int)t[i][j].r > 0){
                    t[i][j].r = (r*5+t[i][j].r*rd)/(rd+5);
                }
                if(g-(int)t[i][j].g > 0){
                    t[i][j].g = (g*5+t[i][j].g*gd)/(gd+5);
                }
                if(b-(int)t[i][j].b > 0){
                    t[i][j].b = (b*5+t[i][j].b*bd)/(bd+5);
                }
            }
        destroy(_imagen,_x,_y);
        _imagen=t;
    }
    void cartoonizer(unsigned char niveles=2, unsigned char tolerancia=10){
        pixel **post;
        fill(post,_x,_y);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                post[i][j] = _imagen[i][j];
        posterizar(niveles);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                post[i][j] = _imagen[i][j];
        carboncillo(tolerancia);
        _imagen = (pixel**)((int)_imagen ^ (int)post);
        post    = (pixel**)((int)_imagen ^ (int)post);
        _imagen = (pixel**)((int)_imagen ^ (int)post);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                if(post[i][j].is(0,0,0))
                    _imagen[i][j].set(0,0,0);
        destroy(post,_x,_y);
    }
    void cartoon(unsigned char niveles=2, unsigned char tolerancia=10){
        pixel **carbon;
        fill(carbon,_x,_y);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                carbon[i][j] = _imagen[i][j];
        posterizar(niveles);
        carboncillo(tolerancia);
        _imagen = (pixel**)((int)_imagen ^ (int)carbon);
        carbon  = (pixel**)((int)_imagen ^ (int)carbon);
        _imagen = (pixel**)((int)_imagen ^ (int)carbon);

        unsigned int **mat = new unsigned int*[_x];
        for(int i=0; i<_x; i++)
            mat[i] = new unsigned int[_y];
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                mat[i][j] = 0;
        unsigned int contador=1;
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                if(carbon[i][j].is(255,255,255))
                    setColorId(i,j,mat,contador);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                if(mat[i][j]!=0){
                    drawColor(getAverage(mat[i][j], mat), mat[i][j], mat);
                }

        destroy(carbon,_x,_y);
    }
    void aumentarLuz(unsigned char t){
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++){
                if(_imagen[i][j].r>=255-t)
                    _imagen[i][j].r=255;
                else _imagen[i][j].r+=t;
                if(_imagen[i][j].g>=255-t)
                    _imagen[i][j].g=255;
                else _imagen[i][j].g+=t;
                if(_imagen[i][j].b>=255-t)
                    _imagen[i][j].b=255;
                else _imagen[i][j].b+=t;
            }
    }

    virtual void draw(sf::RenderTarget& rt, sf::RenderStates rs)const{
        sf::Image img;
        img.create(_x, _y);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                img.setPixel(i,j, sf::Color(_imagen[i][j].r,_imagen[i][j].g,_imagen[i][j].b));
        sf::Texture tx;
        tx.loadFromImage(img);
        rt.draw(sf::Sprite(tx));
    }

private:
    void drawColor(pixel p, unsigned int colorId, unsigned int** mat){
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                if(mat[i][j]==colorId){
                    mat[i][j] = 0;
                    _imagen[i][j] = p;
                }
    }
    pixel getAverage(unsigned int colorId, unsigned int** mat){
        int r,g,b,n;
        r=g=b=n=0;
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                if(mat[i][j]==colorId){
                    r=_imagen[i][j].r;
                    g=_imagen[i][j].g;
                    b=_imagen[i][j].b;
                    ++n;
                }
        if(n==0) return pixel();
        return pixel(r/n,g/n,b/n);
    }
    void setColorId(int x, int y, unsigned int**& mat, unsigned int& contador){
        bool a=false, b=false;
        if(x-1>=0 && mat[x-1][y]!=0) a=true;
        if(y-1>=0 && mat[x][y-1]!=0) b=true;
        if(!a && !b){
            mat[x][y] = contador++;
            return;
        }else if(a && b){
            unsigned int t = mat[x-1][y], t2 = mat[x][y-1];
            if(t!=t2)
                for(int i=0; i<=x; i++)
                    for(int j=0; j<_y; j++)
                        if(mat[i][j]==t)
                            mat[i][j]=t2;
            mat[x][y] = t2;
            return;
        }
        mat[x][y] = (a?mat[x-1][y]:mat[x][y-1]);
    }
};


int main (int argc, char** argv) {
    srand(time(0));
    bool running = true;
    Image img;
    string fileName = "tigre.pbm";
    if(argc==2){
        fileName = argv[1];
    }else{
        string t;
        cout << "Insert fileName (insert nothing for default \"" + fileName + "\"): ";
        getline(cin, t);
        if(t.size()>0){
            if(t.size()>=4 && t.substr(t.size()-4)!=".pbm")
                t += ".pbm";
            fileName = t;
        }
        fileName = "images/" + fileName;
    }
    if(!img.loadFromPBM(fileName)){
        cout << "Couldn't load file..." << endl;
        return 1;
    }

    map<sf::Keyboard::Key, void(*)(Image&)> keyBindings = {
        {sf::Keyboard::Q, [](Image& image){image.posterize(1);}},
        {sf::Keyboard::W, [](Image& image){image.blur(2);}},
        {sf::Keyboard::E, [](Image& image){image.bloom(5);}},
        {sf::Keyboard::R, [](Image& image){image.charcoal(20);}},
        {sf::Keyboard::T, [](Image& image){image.craze();}},
        {sf::Keyboard::A, [](Image& image){image.blackAndWhite();}},
        {sf::Keyboard::S, [](Image& image){image.grayscale();}},
        {sf::Keyboard::D, [](Image& image){image.invert();}}
    };

    cout << "SPACE for save.\nENTER for reopen image.\nQ,W,E,R,T,A,S and D for apply effects." << endl;

    sf::Vector2i mouse;
    sf::RenderWindow window;
    window.create(sf::VideoMode(img.getX(), img.getY()), (const char*)"ImagesEditor");
    window.setFramerateLimit(10);

    sf::Event ev;
    while(running){
        while(running && window.pollEvent(ev)){
            switch(ev.type){
            case sf::Event::MouseMoved:
                mouse.x = ev.mouseMove.x;
                mouse.y = ev.mouseMove.y;
                break;
            case sf::Event::MouseButtonPressed:
                SetFocus(window.getSystemHandle());
                break;
            case sf::Event::Closed:
                running = false;
                break;
            case sf::Event::KeyPressed:
                switch(ev.key.code){
                case sf::Keyboard::Space:
                    if(img.saveToPBM("resultado.pbm"))
                        cout << "Exported to \"resultado.pbm\"." << endl;
                    break;
                case sf::Keyboard::Return:
                    img.loadFromPBM(fileName);
                    break;
                case sf::Keyboard::Escape:
                    running = false;
                    break;
                default:
                    auto it = keyBindings.find(ev.key.code);
                    if(it!=keyBindings.end()){
                        it->second(img);
                    }
                    break;
                }
                break;
            default:
                break;
            }
        }
        window.clear();

        window.draw(img);

        window.display();
    }

    return 0;
}
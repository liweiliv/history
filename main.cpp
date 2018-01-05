/*
 * main.cpp
 *
 *  Created on: 2017年11月17日
 *      Author: liwei
 */
#include <assert.h>
#include <unistd.h>
#include "GL/glew.h"
#ifdef USE_GLFW
#include "GLFW/glfw3.h"
#endif
#include "world.h"
#include "g_map.h"
#include "INIParser.h"
#include "Log_r.h"
#include "config.h"


utility::INIParser g_ini;
GLFWwindow* main_window = NULL;
int8_t main_window_full_screen =0;
int main_window_size_high = 0;
int main_window_size_width = 0;
const char * conf_file = NULL;
typedef int (*key_action)(int action,int mods);

key_action key_action_lsit[512] = {0};

void Key_callback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{
    if(pWindow == main_window)
    {
        assert(key<512);
        if(key_action_lsit[key])
            key_action_lsit[key](action,mods);
    }
    else
        return;//todo
}
void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
void window_refresh_callback(GLFWwindow * w)
{

}
void setWindowsSize_callback(GLFWwindow*,int w,int h)
{
    GLfloat aspectRatio;
    // 防止被0所除

    if (0 == h){
        h = 1;
    }
    // 设置视口为窗口的大小
    glViewport(0, 0, w, h);
    // 选择投影矩阵，并重置坐标系统
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 计算窗口的纵横比（像素比）
    aspectRatio = (GLfloat) w / (GLfloat) h;
    // 定义裁剪区域（根据窗口的纵横比，并使用正投影）
    if (w <=h) {// 宽 < 高
        glOrtho(-1.0, 1.0, -1 /aspectRatio, 1 / aspectRatio, 1.0, -1.0);
    } else {// 宽 > 高
        glOrtho(-1.0 * aspectRatio, 1.0 *aspectRatio, -1.0, 1.0, 1.0, -1.0);
    }
    // 选择模型视图矩阵，并重置坐标系统
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Log_r::Notice("window size change to %d*%d",w,h);
}
void init_glfw_callback()
{
    glfwSetErrorCallback(error_callback);
    glfwSetWindowRefreshCallback(main_window,window_refresh_callback);
    glfwSetKeyCallback(main_window,Key_callback);
    glfwSetWindowSizeCallback(main_window,setWindowsSize_callback);
}

int init()
{
    if(0!=Log_r::Init("/home/liwei/history/log.ini","World","g"))
    {
        fprintf(stderr, "init log failed\n");
        return -1;
    }

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    g_ini.load(conf_file);

    /*获取显示器分辨率*/
    GLFWmonitor * main_monitor = glfwGetPrimaryMonitor();
    if(main_monitor == NULL)
    {
        glfwTerminate();
        return -2;
    }
    const GLFWvidmode * mode = glfwGetVideoMode(main_monitor);
    if(mode == NULL)
    {
        glfwTerminate();
        return -3;
    }
    /*是否全屏幕*/
    main_window_full_screen = g_ini.get_int(G_VIDEO,G_VIDEO_FULL_SCREEN,0);
    if(main_window_full_screen)
    {
        main_window_size_high = mode->height;
        main_window_size_width = mode->width;
        Log_r::Notice("full screen mod,resolution is %d*%d",mode->width,mode->height);
    }
    else
    {
        /*设置分辨率*/
        main_window_size_high=g_ini.get_int(G_VIDEO,G_VIDEO_RESOLUTION_HIGH,-1);
        main_window_size_width=g_ini.get_int(G_VIDEO,G_VIDEO_RESOLUTION_WIDTH,-1);

        if(main_window_size_high <=0||main_window_size_high>mode->height||main_window_size_width<=0||main_window_size_width>mode->width)
        {
            Log_r::Warn("main monitor resolution is %d*%d,in config resolution is %d*%d",mode->width,mode->height,
                    main_window_size_width<=0?0:main_window_size_width,main_window_size_high<=0?0:main_window_size_high);
            main_window_size_high = mode->height*2/3;
            main_window_size_width = mode->width*2/3;
        }
        Log_r::Notice("window mod ,resolution is :%d*%d",main_window_size_width,main_window_size_high);
    }
    return 0;
}
int destroy()
{
    Log_r::Shutdown();
    glfwTerminate();
    return 0;
}
int init_main_window()
{
    if(main_window!=NULL)
        return 0;
    /* Create a windowed mode window and its OpenGL context */
    main_window = glfwCreateWindow(main_window_size_width,main_window_size_high, "World", NULL, NULL);
    if (!main_window)
    {
        Log_r::Error("create window failed");
        return -1;
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(main_window);
    init_glfw_callback();
    return 0;
}
int main_loop()
{
    g_map m(1,0,0,0,0,"fs");
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(main_window))
    {
        usleep(200000);
        glClearColor(1.0, 1.0, 1.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        m.draw(7);
        //w.draw();
        /* Swap front and back buffers */
        glfwSwapBuffers(main_window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    return 0;
}

static bool getOption(int argc, char* argv[])
{
    int c;
    while (true)
    {
        c = getopt(argc, argv, "c:");
        if (c == -1)
            break;
        switch (c)
        {
        case 'c':
            conf_file = optarg;
            break;
        default:
            printf("unknown argv -%c %s\n",c,optarg==NULL?"NULL":optarg);
            break;
        }
    }
    if(conf_file == NULL)
        conf_file = "main.conf";
#if 0
    if(conf_file == NULL)
    {
        printf("usage :Wolrd -c [config_path]\n"
                "-c [config_path] : config_path\n"
                    );
        return false;
    }
#endif
    return true;
}
int main(int argc,char * argv[] )
{
    if(!getOption(argc,argv))
        return -1;
    if(0!=init())
        return -1;
    if(0!=init_main_window())
        goto END;
    main_loop();
END:
    destroy();
    return 0;
}

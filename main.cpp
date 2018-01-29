/*
 * main.cpp
 *
 *  Created on: 2017年11月17日
 *      Author: liwei
 */
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include "GL/glew.h"
#ifdef USE_GLFW
#include "GLFW/glfw3.h"
#endif
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

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

key_action key_action_list[512] = {0};

glm::mat4 transform_camera(1.0f); // 摄像机的位置和定向，即摄像机在世界坐标系中位置
glm::mat4 transform_model(1.0f);  // 模型变换矩阵，即物体坐标到世界坐标
glm::vec4 position_light0(0);     // 光源位置，世界坐标系中的坐标
float speed_scale=0.01f;           // 鼠标交互，移动速度缩放值


int key_up(int action,int mods)
{
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		glm::mat4 mod ;
		transform_camera *= glm::translate(mod,glm::vec3(0.0f,0.0f,speed_scale));
	}
	return 0;
}
int key_down(int action,int mods)
{
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		glm::mat4 mod ;
		transform_camera *= glm::translate(mod,glm::vec3(0.0f,0.0f,-speed_scale));	}
	return 0;
}
int key_right(int action,int mods)
{
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		glm::mat4 mod ;
		transform_camera *= glm::translate(mod,glm::vec3(0.0f,speed_scale,0.0f));
	}
	return 0;
}
int key_left(int action,int mods)
{
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		glm::mat4 mod ;
		transform_camera *= glm::translate(mod,glm::vec3(0.0f,-speed_scale,0.0f));
	}
	return 0;
}
int key_enter(int action,int mods)
{
	if(action == GLFW_PRESS)
	{
		glm::mat4 mod ;
		transform_camera = glm::translate(mod,glm::vec3(0.0f,0.0f,0));	}
	return 0;
}
void init_key_func()
{
	key_action_list[GLFW_KEY_UP] = key_action_list[GLFW_KEY_W] = key_up;
	key_action_list[GLFW_KEY_DOWN] = key_action_list[GLFW_KEY_S] = key_down;
	key_action_list[GLFW_KEY_RIGHT] = key_action_list[GLFW_KEY_A] = key_right;
	key_action_list[GLFW_KEY_LEFT] = key_action_list[GLFW_KEY_D] = key_left;

	key_action_list[GLFW_KEY_ENTER] = key_enter;
}
void Key_callback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{
    if(pWindow == main_window)
    {
        assert(key<512);
        if(key_action_list[key])
            key_action_list[key](action,mods);
    }
    else
        return;//todo
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			break;
		default:
			return;
		}
	return;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//view_source_point[2]+=yoffset/100;
}
void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
void window_refresh_callback(GLFWwindow * w)
{

}
void setWindowsSize_callback(GLFWwindow* window,int w,int h)
{
    GLfloat aspectRatio;
    // 防止被0所除
    if (0 == h)
        h = 1;
    // 设置视口为窗口的大小
    glViewport(0, 0, w, h);
    // 选择投影矩阵，并重置坐标系统
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 计算窗口的纵横比（像素比）
    aspectRatio = (GLfloat) w / (GLfloat) h;
    // 定义裁剪区域（根据窗口的纵横比，并使用正投影）
    if (w <=h) {// 宽 < 高
        glOrtho(-1.0, 1.0, -1 /aspectRatio, 1 / aspectRatio, 100.0, -10.0);
    } else {// 宽 > 高
        glOrtho(-1.0 * aspectRatio, 1.0 *aspectRatio, -1.0, 1.0, 100.0, -10.0);
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
    glfwSetMouseButtonCallback(main_window, mouse_button_callback);
    glfwSetScrollCallback(main_window, scroll_callback);
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
    init_key_func();
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
    setWindowsSize_callback(main_window,main_window_size_width,main_window_size_high);
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(main_window))
    {
        usleep(200000);
        glm::mat4 model_view_matrix = glm::affineInverse(transform_camera);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&model_view_matrix[0][0]);
        glClearColor(1.0, 1.0, 1.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        m.draw(8);

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

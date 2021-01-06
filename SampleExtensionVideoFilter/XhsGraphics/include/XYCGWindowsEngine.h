#pragma once


#include<iostream>
#include<Windows.h>
#include<glad.h>

#define USE_CV_DEBUG
//#define USE_TIME_DEBUG

#ifdef APIEXPORT
#define DllExport __declspec(dllexport)
#else
#define DllExport __declspec(dllimport)
#endif

#include "xhs_mobile_common.h"

//using zs_ai::AiSystem;
namespace CG{
	static const int COLOR_UNKNOWN = 0;
	static const int COLOR_BGR = 2;              // [bgrbgrbgr]
	static const int COLOR_BGRA = 6;             // [bgrabgrabgra]
	static const int COLOR_NV21 = 8;             // NV21 [yyyyvuvu]
	static const int COLOR_NV12 = 9;             // NV12 [yyyyuvuv]
	static const int COLOR_RGBA = 11;            // [rgbargbargba]
	static const int COLOR_RGB = 3;           // [rgbrgbrgb]

class DllExport XYCGWindowsEngine {
public:
	XYCGWindowsEngine();
	~XYCGWindowsEngine();
	
	/// <summary>
	/// ��ʼ��opengl windows�����Լ���Ⱦ����
	/// </summary>
	/// <returns>����0Ϊ�ɹ�������Ϊʧ��</returns> 
	int initWindowsEngine();
	
	/// <summary>
	/// ����opengl windows�����Լ���Ⱦ����
	/// </summary>
	/// <returns>����0Ϊ�ɹ�������Ϊʧ��</returns>
	int destroyWindowsEngine();

	/// <summary>
	/// ����AIģ�ͣ������������
	/// </summary>
	/// <param name="path">ģ��·��</param>
	/// <returns>����0Ϊ�ɹ�������Ϊʧ��</returns>
	int loadAIModel(const char *path);
	
	/// <summary>
	/// ��ȡ���ա��˾��ȴ���������(RGB��ʽ)��������
	/// </summary>
	/// <returns>����ֵΪbuffer����</returns>
	/// 
	void* getOutputData();

	/// <summary>
	/// ��ȡ���ա��˾��ȴ���������(YUV-I420��ʽ)��������
	/// </summary>
	void getOutputYUVData(uint8_t* y, uint8_t* u, uint8_t* v);

	/// <summary>
	/// ÿһ֡�����ݴ���ӿڣ������������+��Ⱦ����ÿһ֡����������Զ�������Ⱦ�ӿ�
	/// </summary>
	/// <param name="data">ԭʼ����</param>
	/// <param name="pixelFormat">ԭʼ���ݸ�ʽ�����COLOR_*</param>
	/// <param name="data_width">ԭʼ��������Ӧ��ͼ��Ŀ�</param>
	/// <param name="data_height">ԭʼ��������Ӧ��ͼ��ĸ�</param>
	/// <param name="stride">ԭʼ���ݵ�stride</param>
	/// <param name="data_len">ԭʼ���ݵ��ֽ���</param>
	void process(unsigned char *data, int pixelFormat, int data_width, int data_height, int stride, int data_len);
	
	/// <summary>
	/// ����yuv����
	/// </summary>
	/// <param name="y"></param>
	/// <param name="u"></param>
	/// <param name="v"></param>
	/// <param name="width"></param>
	/// <param name="height"></param>
	void processYUV(uint8_t* y, uint8_t* u, uint8_t* v, int width, int height);

	/// <summary>
	/// ����������Դ·��
	/// </summary>
	/// <param name="path">��Դ·��</param>
	/// <returns>0Ϊ�ɹ�������Ϊʧ��</returns>
	int setBeautyResourcePath(const char * path);

	/// <summary>
	/// �����˾���Դ·��
	/// </summary>
	/// <param name="path">��Դ·��</param>
	/// <returns>0Ϊ�ɹ�������Ϊʧ��</returns>
	int setFilterResourcePath(const char * path);

	/// <summary>
	/// �Ƿ���һ�����գ�����ĥƤ�����ף������ȣ�
	/// </summary>
	/// <param name="enable">true,������false���رգ�</param>
	void enableBeauty(bool enable);

	/// <summary>
	/// �Ƿ����������գ�ĥƤor����or����
	/// </summary>
	/// <param name="beautyType">�ɼ�����FaceBeautyType</param>
	/// <param name="enable">true,����;false���ر�;</param>
	void enableBeautyType(FaceBeautyType beautyType,bool enable);

	/// <summary>
	/// �Ƿ����˾�
	/// </summary>
	/// <param name="enable">true,����;false,�ر�;</param>
	void enableFilter(bool enable);

	/// <summary>
	/// �Ƿ�����ɫ����
	/// </summary>
	/// <param name="enable">true,����;false,�ر�;</param>
	void enableColorfulAdjustment(bool enable);

	/// <summary>
	/// ���ö���������������Ӧ��ǿ��
	/// </summary>
	/// <param name="beautyType">�ɼ�����FaceBeautyType</param>
	/// <param name="intensity">�����������Ӧ��ǿ��</param>
	void setBeautyTypeIntensity(FaceBeautyType beautyType, float intensity);

	/// <summary>
	/// �����˾�����Ӧ��ǿ��
	/// </summary>
	/// <param name="intensity">ǿ��</param>
	void setFilterIntensity(float intensity);

	/// <summary>
	/// ������ɫ���������Լ�������Ӧ��ǿ��
	/// </summary>
	/// <param name="colorfulType">��ɫ�������ͣ����@xhs_colorful_type</param>
	/// <param name="intensity">ǿ��</param>
	void setColorfulTypeIntensity(xhs_colorful_type colorfulType, float intensity);

	/// <summary>
	/// �����˾�Ч����Ӧ�õ���Ч����
	/// </summary>
	/// <param name="roi_point_x">�����������x���꣬��������</param>
	/// <param name="roi_point_y">�����������y���꣬��������</param>
	/// <param name="roi_width">���������ȣ����ؿ��</param>
	/// <param name="roi_height">��������߶ȣ����ظ߶�</param>
	/// <param name="roi_color_flag">ɫ��Ч���Ƿ�����������ֻ����������1����ȫͼ����0</param>
	/// <param name="roi_picture_flag">�߿�Ч���Ƿ�����������ֻ����������1����ȫͼ����0</param>
	void setFilterIntervalParam(float roi_point_x, float roi_point_y, float roi_width, float roi_height, int roi_color_flag, int roi_picture_flag);

private:
	void render();
private:
	HGLRC m_renderContext;
	GLuint* m_inputTexture = nullptr;
	GLuint* m_tmpTexture = nullptr;
	GLuint* m_outputTexture = nullptr;
	GLuint* m_uploadPboId = nullptr;
	GLuint* m_downloadPboId = nullptr;
	GLuint* m_outputFboId = nullptr;
	int m_inputWidth = 0;
	int m_inputHeight = 0;
	int m_frameIndex = 0;
	bool m_ready = false;
	bool m_beautyEnable = false;
	bool m_filterEnable = false;
	bool m_colorfulEnable = false;
	void* m_inputData = nullptr;
	void* m_outputData = nullptr;
	xhs_handle_t m_filterEngineHandle = nullptr;
	XYFilterRoiParam m_roi = {0};
};
};
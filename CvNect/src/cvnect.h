//	-------------------------------------------------------------------------------------------
//	CvNect v1.0
//
//	Kerem Gocen	- https://github.com/humham
//
//	A project to display Microsoft Kinect's depth and color streams inside OpenCV windows
//	and record individual streams by pressing dedicated keys.
//
//	MS Kinect SDK version:1.8 - OpenCV version:2.4.2	
//	Compiled using Microsoft Visual Studio Express 2012
//
//	required precompiled DLLs: opencv_core242.dll, opencv_highgui242.dll, opencv_imgproc242.dll
//	-------------------------------------------------------------------------------------------

#ifndef _CVNECT_H_
#define _CVNECT_H_

#include <iostream>

#include "NuiApi.h"

#include "opencv2\highgui\highgui.hpp"
#include "opencv2\core\core.hpp"

using namespace cv;

enum writerState_e
{
    W_S_SUCCESS = 0 ,
    W_S_FAIL        ,
    W_S_INVALID      
};

class CvNect
{
    static const int	COLOR_WIDTH		= 640;
    static const int	COLOR_HEIGHT	= 480;
    static const int	DEPTH_WIDTH		= 640;
    static const int	DEPTH_HEIGHT	= 480;
    static const int	CHANNEL			= 3;

public:

    /// <summary>
    /// Constructor
    /// </summary>
    CvNect();

    /// <summary>
    /// Destructor
    /// </summary>
    ~CvNect();

    /// <summary>
    /// Initialize Kinect
    /// </summary>
    /// <returns>S_OK if successful, otherwise an error code</returns>
    HRESULT	CvNect_Init( );

    /// <summary>
    /// Uninitialize Kinect
    /// </summary>
    void	CvNect_UnInit( );

    /// <summary>
    /// Zero out member variables
    /// </summary>
    void	CvNect_Zero( );

    string	StartColorRecording( int fileNumber );
    bool	StopColorRecording();
    string	StartDepthRecording( int fileNumber );
    bool	StopDepthRecording();

private:

    /// <summary>
    /// Thread to handle Kinect processing, calls class instance thread processor
    /// </summary>
    /// <param name="pParam">instance pointer</param>
    /// <returns>always 0</returns>
    static DWORD WINAPI	Nui_ProcessThread( LPVOID pParam );

    /// <summary>
    /// Thread to handle Kinect processing
    /// </summary>
    /// <returns>always 0</returns>
    DWORD WINAPI	Nui_ProcessThread( );

    // thread handling
    HANDLE	m_hThNuiProcess;
    HANDLE	m_hEvNuiProcessStop;

    HANDLE	m_hEvWaitColorWrite;
    HANDLE	m_hEvWaitDepthWrite;

    HANDLE	m_hNextColorFrameEvent;
    HANDLE	m_hNextDepthFrameEvent;

    HANDLE	m_pVideoStreamHandle;
    HANDLE	m_pDepthStreamHandle;

    // OpenCV image buffer and video writers
    IplImage      *color;
    IplImage      *depth;

    VideoWriter	  m_vWriterColor;
    VideoWriter	  m_vWriterDepth;

	CvFont	m_cvFont;

    bool	m_recordColor;
    bool	m_recordDepth;

    int		drawColor();
    int		drawDepth();

    int		m_ColorFramesTotal;
    int		m_LastColorFramesTotal;

    int		m_DepthFramesTotal;
    int		m_LastDepthFramesTotal;

    int		m_fpsColor;
    int		m_fpsDepth;

protected:

    BYTE buf[ DEPTH_WIDTH * DEPTH_HEIGHT * CHANNEL ];

};

#endif /* _CVNECT_H_ */
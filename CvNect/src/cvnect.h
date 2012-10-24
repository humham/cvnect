#ifndef _CVNECT_H_
#define _CVNECT_H_

#include "stdafx.h"

#include <sstream>
#include <string>
#include <mmsystem.h>

#include "common.h"

#include "JointLogger.h"

using namespace cv;

void         mouseCallBack( int event, int x, int y, int flags, void* param );

enum writerState_e
{
    W_S_SUCCESS = 0 ,
    W_S_FAIL        ,
    W_S_INVALID      
};

class CvNect
{
    static const int    COLOR_WIDTH             = 640;
    static const int    COLOR_HEIGHT            = 480;
    static const int    DEPTH_WIDTH             = 640;
    static const int    DEPTH_HEIGHT            = 480;
    static const int    SKELETON_WIDTH          = 640;
    static const int    SKELETON_HEIGHT         = 480;
    static const int    CHANNEL                 = 3;

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
    HRESULT                 CvNect_Init( );

    /// <summary>
    /// Uninitialize Kinect
    /// </summary>
    void                    CvNect_UnInit( );

    /// <summary>
    /// Zero out member variables
    /// </summary>
    void                    CvNect_Zero( );

    // string                  StartColorRecording( int fileNumber );
    // bool                    StopColorRecording();
    string                  StartDepthRecording( int fileNumber );
    bool                    StopDepthRecording();
    string                  StartSkeletonRecording( int fileNumber );
    bool                    StopSkeletonRecording();

    void                    SetMouseColorCoordinates( int x , int y );
private:

    /// <summary>
    /// Thread to handle Kinect processing, calls class instance thread processor
    /// </summary>
    /// <param name="pParam">instance pointer</param>
    /// <returns>always 0</returns>
    static DWORD WINAPI     Nui_ProcessThread( LPVOID pParam );

    /// <summary>
    /// Thread to handle Kinect processing
    /// </summary>
    /// <returns>always 0</returns>
    DWORD WINAPI            Nui_ProcessThread( );

    // thread handling
    HANDLE        m_hThNuiProcess;
    HANDLE        m_hEvNuiProcessStop;

    HANDLE        m_hEvWaitColorWrite;
    HANDLE        m_hEvWaitDepthWrite;
    HANDLE        m_hEvWaitSkeletonWrite;

    HANDLE        m_hNextColorFrameEvent;
    HANDLE        m_hNextDepthFrameEvent;
    HANDLE        m_hNextSkeletonFrameEvent;

    HANDLE        m_pVideoStreamHandle;
    HANDLE        m_pDepthStreamHandle;
    HANDLE        m_pSkeletonStreamHandle;

    DWORD         m_DepthStreamFlags;

    // OpenCV image buffer and video writers
    IplImage      *color;
    IplImage      *depth;
    IplImage      *skeleton;

    // VideoWriter	  vWriterColor;
    VideoWriter	  vWriterDepth;
    VideoWriter   vWriterSkeleton;

    //bool          m_recordColor;
    bool          m_recordDepth;
    bool          m_recordSkeleton;

    int           drawColor();
    int           drawDepth();
    int           drawSkeleton( DWORD time );

    int           m_ColorFramesTotal;
    int           m_LastColorFramesTotal;

    int           m_DepthFramesTotal;
    DWORD         m_LastDepthFPStime;
    int           m_LastDepthFramesTotal;

    CvPoint       pt[ NUI_SKELETON_POSITION_COUNT ];

    int           m_SkeletonFramesTotal;
    int           m_LastSkeletonFramesTotal;
    DWORD         m_LastDrawSkeletonTime;

    NUI_SKELETON_FRAME m_SkeletonFrame;
    CvScalar      m_color_skeletonColorJointSelected;
    CvScalar      m_color_skeletonColorGreen;

    CvFont        m_font;
    double        m_hScale;
    double        m_vScale;
    int           m_lineWidth;

    int           m_fpsColor;
    int           m_fpsDepth;
    int           m_fpsSkeleton;

    int           m_mouseColorX;
    int           m_mouseColorY;

    class   JointLogger        *m_logger;
    void    sendSkeletonDataToJointLogger( DWORD time );

    // function to calculate 3d length between 2 given joints
    double  length( Vector4 j1 , Vector4 j2 );

    // determine which leg is tracked better
    int     numberOfTrackedJoints( NUI_SKELETON_POSITION_TRACKING_STATE state[] , int size );

    // determine height of tracked skeleton
    double  calculateSkeletonHeight( int skeletonIndex );

    char        m_heightText[ 6 ][ 5 ];
    CvPoint     m_heightTextPositions[ 6 ];
    double      m_skeletonHeight[ 6 ];

    Vector4     m_headTopPosition[ 6 ];

    Vector4     m_pointCurrent , m_pointNext;

    Vector4     m_jointPositionsLast30Frames[ 30 ];
    CvPoint     m_jointPositionsLast30PointArray[ 30 ];
    int         m_jointPositionLast30FrameCount;

    void        drawPolyLineAtJointPositions();

    double  calculateSpeedOfSelectedJoint();
    double  m_jointSpeed;

protected:

    BYTE buf[ DEPTH_WIDTH * DEPTH_HEIGHT * CHANNEL ];

};

#endif /* _CVNECT_H_ */

//template <typename ... Ts>
//void format_string(char *fmt, Ts ... ts) {}
//
//template <typename ... Ts>
//void debug_print(int dbg_lvl, char *fmt, Ts ... ts)
//{
//  format_string(fmt, ts...);
//}
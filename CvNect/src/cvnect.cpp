#include "cvnect.h"
#include <cmath>
#include <cstdio>

using namespace cv;
using namespace std;


/**********************************
*
*
**********************************/
void mouseCallBack( int event, int x, int y, int flags, void* param )
{
    IplImage* image = (IplImage*) param;

    switch( event ){
    case CV_EVENT_MOUSEMOVE: 
        //SetMouseColorCoordinates( x , y );
        break;

    case CV_EVENT_LBUTTONDOWN:
        cout << "mouse left button down!" << endl;
        break;

    case CV_EVENT_LBUTTONUP:
        cout << "mouse left button up!" << endl;
        break;

    case CV_EVENT_LBUTTONDBLCLK:
        cout << "mouse double click!" << endl;
        break;

    case CV_EVENT_RBUTTONDOWN:
        cout << "mouse right button down!" << endl;
        break;

    case CV_EVENT_RBUTTONUP:
        cout << "mouse right button up!" << endl;
        break;
    }
}

/**********************************
*
*
**********************************/
void mouseCallBackSkeletonJointWrapper( int event, int x, int y, int flags, void* param )
{
    switch( event )
        {
        case CV_EVENT_LBUTTONDOWN:
             cout << "x:" << x << " y:" << y << endl;
            for( int i=0; i < NUI_SKELETON_POSITION_COUNT; i++ )
            {
                if( ( x >= common::jointPositions[ i ].x - 3 && x <= common::jointPositions[ i ].x + 3 )  && ( y >= common::jointPositions[ i ].y - 3 && y <= common::jointPositions[ i ].y + 3 ) )
                {
                    cout << "mouseCallBackSkeletonJoint joint hit!" << endl;
                    if( common::selectedSkeletonJoint[ i ] )
                    {
                        common::selectedSkeletonJoint[ i ] = FALSE;
                        common::jointSelected = FALSE;
                    }
                    else
                    {
                        common::selectedSkeletonJoint[ i ] = TRUE;

                        if( common::jointSelected == FALSE )
                        {
                            common::selectedJointIndex = i;
                            common::jointSelected = TRUE;
                        }
                    }
                    break;
                }
            }

            break;
        }
}


/**********************************
*
*
**********************************/
CvNect::CvNect()
{
    m_logger = new JointLogger();
    m_color_skeletonColorGreen = CV_RGB( 0 , 255 , 0 );
    m_color_skeletonColorJointSelected = CV_RGB( 255 , 0 , 0 );

    memset( (void *)common::selectedSkeletonJoint , 0 , sizeof( common::selectedSkeletonJoint ) );
    memset( (void *)common::jointPositions , 0 , sizeof( common::jointPositions ) );
}


/**********************************
*
*
**********************************/
CvNect::~CvNect()
{
    CvNect_UnInit();
}


/**********************************
*
*
**********************************/
HRESULT	CvNect::CvNect_Init()
{
    string jointName = "selected_joints";
    m_logger->JointLogger_Init( jointName , NUI_SKELETON_POSITION_HAND_LEFT );

    //color       = cvCreateImageHeader( cvSize( COLOR_WIDTH , COLOR_HEIGHT )         , IPL_DEPTH_8U , 4       );
    depth       = cvCreateImageHeader( cvSize( DEPTH_WIDTH , DEPTH_HEIGHT )         , IPL_DEPTH_8U , CHANNEL );
    skeleton    = cvCreateImage( cvSize( SKELETON_WIDTH , SKELETON_HEIGHT )         , IPL_DEPTH_8U , CHANNEL );

    //cvNamedWindow( "RGB"	, CV_WINDOW_AUTOSIZE );
    cvNamedWindow( "Depth"	, CV_WINDOW_AUTOSIZE );
    cvNamedWindow( "Skeleton"	, CV_WINDOW_AUTOSIZE );

    //cvSetMouseCallback( "RGB" , mouseCallBack , &color );
    cvSetMouseCallback( "Skeleton" , mouseCallBackSkeletonJointWrapper , &skeleton );

    HRESULT hr = NuiInitialize( NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON );

    if( hr != S_OK )
    {
        cout << "NuiInitialize failed " << hr << endl;

        return hr;
    }
    else
    {
        cout << "NuiInitialize successful..\nOpening streams..." << endl;
    }

    /*cout << "Image stream opening..";

    m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    hr = NuiImageStreamOpen( NUI_IMAGE_TYPE_COLOR , NUI_IMAGE_RESOLUTION_640x480 , 0 , 2 , m_hNextColorFrameEvent , &m_pVideoStreamHandle );

    if( FAILED( hr ) )
    {
        cout << "Could not open image stream video" << endl;

        return hr;
    }
    else
    {
        cout << "success.." << endl;
    }*/

    cout << "Depth stream opening..";

    m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    hr = NuiImageStreamOpen( 
        NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX ,
        NUI_IMAGE_RESOLUTION_640x480 ,
        0 ,
        NUI_IMAGE_STREAM_FRAME_LIMIT_MAXIMUM ,
        m_hNextDepthFrameEvent ,
        &m_pDepthStreamHandle );

    if( FAILED( hr ) )
    {
        cout << "Could not open depth stream video" << endl;

        return hr;
    }
    else
    {
        cout << "success.." << endl;
    }


    cout << "Skeleton stream opening..";

    m_hNextSkeletonFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    hr = NuiSkeletonTrackingEnable( m_hNextSkeletonFrameEvent , NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE );

    if( FAILED( hr ) )
    {
        cout << "Could not open skeleton stream video" << endl;

        return hr;
    }
    else
    {
        cout << "success.." << endl;
    }

    m_hScale    = 1.0;
    m_vScale    = 1.0;
    m_lineWidth = 1;

    cvInitFont( &m_font , CV_FONT_HERSHEY_PLAIN , m_hScale , m_vScale , 0 , m_lineWidth );

    m_fpsColor      = 0;
    m_fpsDepth      = 0;
    m_fpsSkeleton   = 0;

    m_mouseColorX   = 0;
    m_mouseColorY   = 0;

    //m_hEvWaitColorWrite = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hEvWaitDepthWrite     = CreateEvent( NULL , FALSE , FALSE , NULL );
    m_hEvWaitSkeletonWrite  = CreateEvent( NULL , FALSE , FALSE , NULL );

    // Start the Nui processing thread
    m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hThNuiProcess     = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );

    return hr;
}


/**********************************
*
*
**********************************/
void	CvNect::CvNect_UnInit()
{
    // Stop the Nui processing thread
    if ( NULL != m_hEvNuiProcessStop )
    {
        // Signal the thread
        SetEvent(m_hEvNuiProcessStop);

        // Wait for thread to stop
        if ( NULL != m_hThNuiProcess )
        {
            WaitForSingleObject( m_hThNuiProcess, INFINITE );
            CloseHandle( m_hThNuiProcess );
        }
        CloseHandle( m_hEvNuiProcessStop );
    }

    if ( m_hNextColorFrameEvent && ( m_hNextColorFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextColorFrameEvent );
        m_hNextColorFrameEvent = NULL;
    }

    if ( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }

    if ( m_hNextSkeletonFrameEvent && ( m_hNextSkeletonFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonFrameEvent );
        m_hNextSkeletonFrameEvent = NULL;
    }

    cvReleaseImageHeader(&color);
    cvReleaseImageHeader( &depth );
    cvReleaseImageHeader( &skeleton );


    /*if( vWriterColor.isOpened() )
    {
    vWriterColor.release();
    }*/
    if( vWriterDepth.isOpened() )
    {
        vWriterDepth.release();
    }
    if( vWriterSkeleton.isOpened() )
    {
        vWriterSkeleton.release();
    }

    cvDestroyWindow("RGB");
    cvDestroyWindow( "Depth" );
    cvDestroyWindow( "Skeleton" );

    NuiShutdown();

}


/**********************************
*
*
**********************************/
void	CvNect::CvNect_Zero()
{
    m_hThNuiProcess             = NULL;
    m_hEvNuiProcessStop         = NULL;

    m_hEvWaitColorWrite         = NULL;
    m_hEvWaitDepthWrite         = NULL;
    m_hEvWaitSkeletonWrite      = NULL;

    m_hNextColorFrameEvent      = NULL;
    m_hNextDepthFrameEvent      = NULL;
    m_hNextSkeletonFrameEvent   = NULL;

    m_pVideoStreamHandle        = NULL;
    m_pDepthStreamHandle        = NULL;
    m_pSkeletonStreamHandle     = NULL;

    m_DepthFramesTotal          = 0;
    m_LastDepthFPStime          = 0;
    m_LastDepthFramesTotal      = 0;

    m_DepthStreamFlags          = 0;

    // m_recordColor           = false;
    m_recordDepth               = false;
    m_recordSkeleton            = false;

    m_ColorFramesTotal          = 0;
    m_LastColorFramesTotal      = 0;

    m_DepthFramesTotal          = 0;
    m_LastDepthFPStime          = 0;
    m_LastDepthFramesTotal      = 0;

    m_SkeletonFramesTotal       = 0;
    m_LastSkeletonFramesTotal   = 0;
    m_LastDrawSkeletonTime      = 0;

    memset( m_heightText , 0 , sizeof( m_heightText ) );
    memset( m_heightTextPositions , 0 , sizeof( m_heightTextPositions ) );
    memset( m_skeletonHeight , 0 , sizeof( m_skeletonHeight ) );

    memset( m_headTopPosition , 0 , sizeof( m_headTopPosition ) );

    m_pointCurrent.x = 0.0;
    m_pointCurrent.y = 0.0;
    m_pointCurrent.z = 0.0;
    m_pointNext.x = 0.0;
    m_pointNext.y = 0.0;
    m_pointNext.z = 0.0;

    common::jointSelected = FALSE;

    m_jointPositionLast30FrameCount = 0;
    memset( m_jointPositionsLast30Frames , 0 , sizeof( m_jointPositionsLast30Frames ) );
    memset( m_jointPositionsLast30PointArray , 0 , sizeof( m_jointPositionsLast30PointArray ) );

    cout << "size of array 30 " << sizeof( m_jointPositionsLast30Frames ) << " " <<  sizeof( m_jointPositionsLast30PointArray ) << endl;
    
    m_jointSpeed = 0.0;
}


/**********************************
*
*
**********************************/
//string CvNect::StartColorRecording( int fileNumber )
//{
//    stringstream fileName;
//    
//    fileName << "colorout" << fileNumber << ".avi";
//
//    string colorFileName = fileName.str();
//
//    if( !vWriterColor.isOpened() )
//    {
//        if( vWriterColor.open( colorFileName , CV_FOURCC( 'I','4','2','0' ) , 30 , cvSize( COLOR_WIDTH , COLOR_HEIGHT ) ) )
//        {
//            m_recordColor = true;
//        }
//        else
//        {
//            return "Failed to open vWriterColor!";
//        }
//    }
//    else
//    {
//        return "vWriterColor is already open!";
//    }
//
//    stringstream outputStream;
//
//    outputStream << "Recording color to " << colorFileName;
//
//    string output = outputStream.str();
//
//    return output;
//}


/**********************************
*
*
**********************************/
//bool CvNect::StopColorRecording()
//{
//    if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hEvWaitColorWrite, INFINITE ) )
//    {
//        if( vWriterColor.isOpened() )
//        {
//            m_recordColor = false;
//
//            vWriterColor.release();
//
//            return 0;
//        }
//    }
//    
//
//    return 1;
//}


/**********************************
*
*
**********************************/
string CvNect::StartDepthRecording( int fileNumber )
{
    stringstream fileName;

    fileName << "depthout" << fileNumber << ".avi";

    string depthFileName = fileName.str();

    if( !vWriterDepth.isOpened() )
    {
        if( vWriterDepth.open( depthFileName , CV_FOURCC( 'I','4','2','0' ) , 30 , cvSize( DEPTH_WIDTH , DEPTH_HEIGHT ) ) )
        {
            m_recordDepth = true;
        }
        else
        {
            return "Failed to open vWriterDepth!";
        }
    }
    else
    {
        return "vWriterDepth is already open!";
    }

    stringstream outputStream;

    outputStream << "Recording depth to " << depthFileName;

    string output = outputStream.str();

    return output;
}


/**********************************
*
*
**********************************/
bool CvNect::StopDepthRecording()
{
    if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hEvWaitDepthWrite , INFINITE ) )
    {
        if( vWriterDepth.isOpened() )
        {
            m_recordDepth = false;

            vWriterDepth.release();

            return 0;
        }
    }

    return 1;
}


/// <summary>
/// Thread to handle Kinect processing, calls class instance thread processor
/// </summary>
/// <param name="pParam">instance pointer</param>
/// <returns>always 0</returns>
DWORD WINAPI CvNect::Nui_ProcessThread( LPVOID pParam )
{
    CvNect *pthis = (CvNect *)pParam;
    return pthis -> Nui_ProcessThread( );
}

/// <summary>
/// Thread to handle Kinect processing
/// </summary>
/// <returns>always 0</returns>
DWORD WINAPI CvNect::Nui_ProcessThread( )
{
    const int numEvents = 3;
    HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop , m_hNextDepthFrameEvent , m_hNextSkeletonFrameEvent };
    int    nEventIdx;
    DWORD  t;

    m_LastDepthFPStime = timeGetTime( );

    cout << "Nui_ProcessThread starting main loop.." << endl;

    // Main thread loop
    bool continueProcessing = true;
    while ( continueProcessing )
    {
        t = timeGetTime( );

        // Wait for any of the events to be signalled
        nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, 1000 );

        // Timed out, continue
        if ( nEventIdx == WAIT_TIMEOUT )
        {
            cout << "Timed out, continue" << endl;
            continue;
        }

        // stop event was signalled 
        if ( WAIT_OBJECT_0 == nEventIdx )
        {
            cout << "stop event was signalled" << endl;
            continueProcessing = false;
            break;
        }

        // Wait for each object individually with a 0 timeout to make sure to
        // process all signalled objects if multiple objects were signalled
        // this loop iteration

        // In situations where perfect correspondance between color/depth/skeleton
        // is essential, a priority queue should be used to service the item
        // which has been updated the longest ago

        if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hNextDepthFrameEvent, 30 ) )
        {
            //only increment frame count if a frame was successfully drawn
            //cout << "got depth frame" << endl;
            if ( drawDepth() == 0 )
            {
                ++m_DepthFramesTotal;
            }
        }

        //if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hNextColorFrameEvent, 30 ) )
        //{
        //    //cout << "got color frame" << endl;
        //    if ( drawColor() == 0 )
        //    {
        //        ++m_ColorFramesTotal;
        //    }
        //}

        if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hNextSkeletonFrameEvent, 30 ) )
        {
            //cout << "got color frame" << endl;
            if ( drawSkeleton( t ) == 0 )
            {
                ++m_SkeletonFramesTotal;
            }
        }

        // Once per second, display the depth FPS
        
        if ( (t - m_LastDepthFPStime) > 1000 )
        {
            m_fpsDepth = ((m_DepthFramesTotal - m_LastDepthFramesTotal) * 1000 + 500) / (t - m_LastDepthFPStime);

            /*m_fpsColor = ((m_ColorFramesTotal - m_LastColorFramesTotal) * 1000 + 500) / (t - m_LastDepthFPStime);*/

            m_fpsSkeleton = ((m_SkeletonFramesTotal - m_LastSkeletonFramesTotal) * 1000 + 500) / (t - m_LastDepthFPStime);
            //PostMessageW( m_hWnd, WM_USER_UPDATE_FPS, IDC_FPS, fps );
            //cout << " m_DepthFramesTotal: " << m_DepthFramesTotal << " m_LastDepthFramesTotal: " << m_LastDepthFramesTotal << " m_LastDepthFPStime: " << m_LastDepthFPStime << endl;
            //cout << "fps_depth: " << fpsDepth << " - fps_color: " << fpsColor << " - fps_skelton: " << fpsSkeleton << endl;

            m_LastDepthFramesTotal = m_DepthFramesTotal;
            m_LastDepthFPStime = t;

            /*m_LastColorFramesTotal = m_ColorFramesTotal;*/

            m_LastSkeletonFramesTotal = m_SkeletonFramesTotal;
        }
    }

    return 0;
}


/**********************************
*
*
**********************************/
int CvNect::drawColor()
{

    const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame( m_pVideoStreamHandle , 0 , &pImageFrame );

    if( FAILED( hr ) )
    {
        cout<<"Get Image Frame Failed"<<endl;

        return -1;
    }

    INuiFrameTexture * pTexture = pImageFrame -> pFrameTexture;

    NUI_LOCKED_RECT LockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );

    if( LockedRect.Pitch != 0 )
    {
        BYTE * pBuffer = (BYTE*) LockedRect.pBits;

        cvSetData( color , pBuffer , LockedRect.Pitch );

        ResetEvent( m_hEvWaitColorWrite );

        /*if( vWriterColor.isOpened() && m_recordColor )
        {
        Mat iplimg( color , true );

        vWriterColor.write( color );
        }*/

        SetEvent( m_hEvWaitColorWrite );
    }

    char fps[3];

    sprintf_s( fps, sizeof( m_fpsColor ) , "%d" , m_fpsColor );

    cvPutText( color, fps , cvPoint( 10 , 20 ) , &m_font , cvScalar( 255 , 255 , 255 ) );

    cvShowImage("RGB",color);

    NuiImageStreamReleaseFrame( m_pVideoStreamHandle , pImageFrame );

    return 0;
}


/**********************************
*
*
**********************************/
int CvNect::drawDepth()
{
    const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame( m_pDepthStreamHandle, 0, &pImageFrame );

    if( FAILED( hr ) )
    {
        cout<<"Get Image Frame Failed"<<endl;

        return -1;
    }

    INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;

    NUI_LOCKED_RECT LockedRect;

    pTexture->LockRect( 0, &LockedRect, NULL, 0 );

    if( LockedRect.Pitch != 0 )
    {
        USHORT * pBuff = (USHORT*) LockedRect.pBits;

        for(int i=0;i<DEPTH_WIDTH*DEPTH_HEIGHT;i++)
        {
            BYTE index = pBuff[i]&0x07;

            USHORT realDepth = ( pBuff[i] & 0xFFF8 )>> 3;

            BYTE scale = 255 - ( BYTE )( 256 * realDepth / 0x0fff);

            buf[CHANNEL*i] = buf[ CHANNEL * i+1 ] = buf[ CHANNEL * i+2 ] = 0;

            switch( index )
            {
            case 0:

                buf[CHANNEL*i]=scale/2;
                buf[CHANNEL*i+1]=scale/2;
                buf[CHANNEL*i+2]=scale/2;

                break;

            case 1:

                buf[CHANNEL*i]=scale;

                break;

            case 2:

                buf[CHANNEL*i+1]=scale;

                break;

            case 3:

                buf[CHANNEL*i+2]=scale;

                break;

            case 4:

                buf[CHANNEL*i]=scale;
                buf[CHANNEL*i+1]=scale;

                break;

            case 5:

                buf[CHANNEL*i]=scale;
                buf[CHANNEL*i+2]=scale;

                break;

            case 6:

                buf[CHANNEL*i+1]=scale;
                buf[CHANNEL*i+2]=scale;

                break;

            case 7:

                buf[CHANNEL*i]=255-scale/2;
                buf[CHANNEL*i+1]=255-scale/2;
                buf[CHANNEL*i+2]=255-scale/2;

                break;
            }

        }

        cvSetData( depth , buf , DEPTH_WIDTH * CHANNEL );

        ResetEvent( m_hEvWaitDepthWrite );

        if( vWriterDepth.isOpened() && m_recordDepth == true )
        {
            Mat iplimg( depth , true );

            vWriterDepth.write( depth );
        }

        SetEvent( m_hEvWaitDepthWrite );

    }

    NuiImageStreamReleaseFrame( m_pDepthStreamHandle , pImageFrame );

    char fps[3];

    sprintf_s( fps, sizeof( m_fpsDepth ) , "%d" , m_fpsDepth );

    cvPutText( depth, fps , cvPoint( 10 , 20 ) , &m_font , cvScalar( 255 , 255 , 255 ) );

    cvShowImage("Depth",depth);

    return 0;
}


/**********************************
*
*
**********************************/
int CvNect::drawSkeleton( DWORD time )
{

    HRESULT hr = NuiSkeletonGetNextFrame( 0, &m_SkeletonFrame );

    bool bFoundSkeleton = false;

    for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
    {
        if( m_SkeletonFrame.SkeletonData[ i ].eTrackingState == NUI_SKELETON_TRACKED )
        {
            bFoundSkeleton = true;
        }

    }

    // Has skeletons!

    if( bFoundSkeleton )
    {
        NuiTransformSmooth(&m_SkeletonFrame,NULL);

        memset(skeleton->imageData,0,skeleton->imageSize);

        for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
        {

            if( m_SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
            {

                for(int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
                {

                    float fx,fy;

                    NuiTransformSkeletonToDepthImage( m_SkeletonFrame.SkeletonData[ i ].SkeletonPositions[ j ] , &fx , &fy );

                    /*cout << "fx: " << fx << " fy: " << fy << endl;*/

                    pt[ j ].x = ( int ) ( fx + SKELETON_WIDTH / 3 );
                    pt[ j ].y = ( int ) ( fy + SKELETON_HEIGHT / 3 );

                    common::jointPositions[ j ].x = pt[ j ].x;
                    common::jointPositions[ j ].y = pt[ j ].y;

                    /*cout << "pt[ " << j << " ].x: " << pt[ j ].x << " pt[ " << j << " ].y: " << pt[ j ].y << endl;*/
                    if( common::selectedSkeletonJoint[ j ] )
                    {
                        cvCircle( skeleton , pt[j] , 5 , m_color_skeletonColorJointSelected , -1 );

                        m_jointPositionsLast30Frames[ m_jointPositionLast30FrameCount ] = m_SkeletonFrame.SkeletonData[ i ].SkeletonPositions[ j ];
                        m_jointPositionsLast30PointArray[ m_jointPositionLast30FrameCount ] = cvPoint( pt[ j ].x , pt[ j ].y );

                        if( m_jointPositionLast30FrameCount == 29 )
                        {
                            // reset index
                            m_jointPositionLast30FrameCount = 0;
                        }
                        else
                        {
                            m_jointPositionLast30FrameCount++;
                        }
                    }
                    else
                    {
                        cvCircle( skeleton , pt[j] , 5 , m_color_skeletonColorGreen , -1 );
                    }
                    
                }

                

                m_heightTextPositions[ i ] = pt[NUI_SKELETON_POSITION_HEAD];

                cvLine(skeleton,pt[NUI_SKELETON_POSITION_HEAD],pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],pt[NUI_SKELETON_POSITION_SPINE],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_SPINE],pt[NUI_SKELETON_POSITION_HIP_CENTER],CV_RGB(0,255,0));

                cvLine(skeleton,pt[NUI_SKELETON_POSITION_HAND_RIGHT],pt[NUI_SKELETON_POSITION_WRIST_RIGHT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_WRIST_RIGHT],pt[NUI_SKELETON_POSITION_ELBOW_RIGHT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_ELBOW_RIGHT],pt[NUI_SKELETON_POSITION_SHOULDER_RIGHT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_SHOULDER_RIGHT],pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],CV_RGB(0,255,0));

                cvLine(skeleton,pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],pt[NUI_SKELETON_POSITION_SHOULDER_LEFT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_SHOULDER_LEFT],pt[NUI_SKELETON_POSITION_ELBOW_LEFT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_ELBOW_LEFT],pt[NUI_SKELETON_POSITION_WRIST_LEFT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_WRIST_LEFT],pt[NUI_SKELETON_POSITION_HAND_LEFT],CV_RGB(0,255,0));

                cvLine(skeleton,pt[NUI_SKELETON_POSITION_HIP_CENTER],pt[NUI_SKELETON_POSITION_HIP_RIGHT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_HIP_RIGHT],pt[NUI_SKELETON_POSITION_KNEE_RIGHT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_KNEE_RIGHT],pt[NUI_SKELETON_POSITION_ANKLE_RIGHT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_ANKLE_RIGHT],pt[NUI_SKELETON_POSITION_FOOT_RIGHT],CV_RGB(0,255,0));

                cvLine(skeleton,pt[NUI_SKELETON_POSITION_HIP_CENTER],pt[NUI_SKELETON_POSITION_HIP_LEFT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_HIP_LEFT],pt[NUI_SKELETON_POSITION_KNEE_LEFT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_KNEE_LEFT],pt[NUI_SKELETON_POSITION_ANKLE_LEFT],CV_RGB(0,255,0));
                cvLine(skeleton,pt[NUI_SKELETON_POSITION_ANKLE_LEFT],pt[NUI_SKELETON_POSITION_FOOT_LEFT],CV_RGB(0,255,0));

                if( time - m_LastDrawSkeletonTime > 1000 )
                {
                    sendSkeletonDataToJointLogger( time );

                    //// every 0.5 second, check for positions of the selected joint and calculate speed
                    //if( common::jointSelected == TRUE )
                    //{
                    //    m_pointNext = m_SkeletonFrame.SkeletonData[ i ].SkeletonPositions[ common::selectedJointIndex ];

                    //    float cx, cy , nx , ny;

                    //    NuiTransformSkeletonToDepthImage( m_pointCurrent , &cx , &cy );
                    //    NuiTransformSkeletonToDepthImage( m_pointNext , &nx , &ny );

                    //    CvPoint current = cvPoint( cx + SKELETON_WIDTH / 3 , cy + SKELETON_HEIGHT / 3 );
                    //    CvPoint next = cvPoint( nx + SKELETON_WIDTH / 3 , ny + SKELETON_HEIGHT / 3 );

                    //    cvCircle( skeleton , current , 5 , CV_RGB( 255 , 255 , 255 ) , -1 );
                    //    cvCircle( skeleton , next , 5 , CV_RGB( 255 , 255 , 255 ) , -1 );
                    //    cvLine( skeleton , current , next , CV_RGB(255,255,255) );

                    //    // min and max points of a selected joint for the last 0.5 sec is acquired, now calculate speed
                    //    calculateSpeedOfSelectedJoint();

                    //    m_pointCurrent = m_pointNext;
                    //}

                    m_LastDrawSkeletonTime = time;
                }

                // calculate and display height every second
                if( time - m_LastDepthFPStime > 1000 )
                {
                    m_skeletonHeight[ i ] = calculateSkeletonHeight( i );

                    memset( (void *)m_heightText , 0 , sizeof( m_heightText[ i ] ) );
                    
                    sprintf_s( m_heightText[ i ] , sizeof( m_skeletonHeight[ i ] ) , "%.2lf" , m_skeletonHeight[ i ] );

                    cout << m_skeletonHeight[ i ] << " text:" << m_heightText[ i ] << " i:" << i << endl;
                }

                if( m_jointPositionLast30FrameCount > 0 && common::jointSelected )
                {
                    drawPolyLineAtJointPositions();
                }

                cvPutText( skeleton , m_heightText[ i ] , cvPoint( m_heightTextPositions[ i ].x , m_heightTextPositions[ i ].y - 20 ) , &m_font , cvScalar( 255 , 255 , 255 ) );
            }
        }
       
    }

    char fps[3];

    sprintf_s( fps, sizeof( m_fpsSkeleton ) , "%d" , m_fpsSkeleton );

    cvPutText( skeleton, fps , cvPoint( 10 , 20 ) , &m_font , cvScalar( 255 , 255 , 255 ) );

    if( common::jointSelected )
    {
        char speed[ 4 ];

        sprintf_s( speed, sizeof( m_jointSpeed ) , "%.3lf" , m_jointSpeed );

        cvPutText( skeleton, speed , cvPoint( SKELETON_WIDTH - 50 , 50 ) , &m_font , cvScalar( 255 , 255 , 255 ) );
    }
   
    cvShowImage( "Skeleton" , skeleton );

    return 0;
}


/**********************************
* Invoked by drawSkeleton, sends 
* static joint points to JointLogger object
**********************************/
void   CvNect::sendSkeletonDataToJointLogger( DWORD time )
{
    if( m_logger != NULL )
    {
        m_logger -> LogData( time , common::selectedSkeletonJoint , common::jointPositions );
    }
}


/**********************************
*
*
**********************************/
void     CvNect::SetMouseColorCoordinates( int x , int y )
{
    m_mouseColorX = x;
    m_mouseColorY = y;
}


/**********************************
*
*
**********************************/
double CvNect::length( Vector4 j1 , Vector4 j2 )
{
    return sqrtf( pow( j1.x - j2.x , 2 ) + pow( j1.y - j2.y , 2 ) + pow( j1.z - j2.z , 2 ) );
}


/**********************************
*
*
**********************************/
int     CvNect::numberOfTrackedJoints( NUI_SKELETON_POSITION_TRACKING_STATE state[] , int size )
{
    int trackedJoints = 0;

    for(  int i = 0; i < size; i++ )
    {
        if( state[ i ] == NUI_SKELETON_POSITION_TRACKED)
        {
            trackedJoints++;
        }
    }

    return trackedJoints;
}


/**********************************
*
*
**********************************/
double CvNect::calculateSkeletonHeight( int skeletonIndex )
{
    // distance from head joint to the top of head
    const double head_divergence = 0.1;

    double bodyLength   = 0.0;

    // check if whole body is tracked, return 0 otherwise
    if( m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_HEAD ] == NUI_SKELETON_POSITION_TRACKED &&
        m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_SHOULDER_CENTER ] == NUI_SKELETON_POSITION_TRACKED &&
        m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_SPINE ] == NUI_SKELETON_POSITION_TRACKED &&
        m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_HIP_CENTER ] == NUI_SKELETON_POSITION_TRACKED )
    {
        // add head to shoulder length
        bodyLength += length( m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_HEAD ] , m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_SHOULDER_CENTER ] );
        // add shoulder to spine length
        bodyLength += length( m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_SHOULDER_CENTER ] , m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_SPINE ] );
        // add spine to hip length
        bodyLength += length( m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_SPINE ] , m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_HIP_CENTER ] );
    }
    else
    {
        return 0.0;
    }

    double legLength    = 0.0;

    NUI_SKELETON_POSITION_TRACKING_STATE leftState[] = { m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_HIP_LEFT ] ,
        m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_KNEE_LEFT ] ,
        m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_ANKLE_LEFT ] ,
        m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_FOOT_LEFT ] };

    NUI_SKELETON_POSITION_TRACKING_STATE rightState[] = { m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_HIP_RIGHT ] ,
        m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_KNEE_RIGHT ] ,
        m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_ANKLE_RIGHT ] ,
        m_SkeletonFrame.SkeletonData[ skeletonIndex ].eSkeletonPositionTrackingState[ NUI_SKELETON_POSITION_FOOT_RIGHT ] };

    int trackedJointsLeftleg    = numberOfTrackedJoints( leftState , 4 );
    int trackedJointsRightleg   = numberOfTrackedJoints( rightState , 4 );

    // calculate leg lengt depending on which leg is tracked better

    if( trackedJointsLeftleg > trackedJointsRightleg && trackedJointsLeftleg == 4 )
    {
        // add hip to knee length
        legLength += length( m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_HIP_LEFT ] , m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_KNEE_LEFT ] );
        // add knee to ankle length
        legLength += length( m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_KNEE_LEFT ] , m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_ANKLE_LEFT ] );
        // add ankle to foot length
        legLength += length( m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_ANKLE_LEFT ] , m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_FOOT_LEFT ] );
    }
    else if( trackedJointsRightleg == 4 )
    {
        // add hip to knee length
        legLength += length( m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_HIP_RIGHT ] , m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_KNEE_RIGHT ] );
        // add knee to ankle length
        legLength += length( m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_KNEE_RIGHT ] , m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_ANKLE_RIGHT ] );
        // add ankle to foot length
        legLength += length( m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_ANKLE_RIGHT ] , m_SkeletonFrame.SkeletonData[ skeletonIndex ].SkeletonPositions[ NUI_SKELETON_POSITION_FOOT_RIGHT ] );
    }
    else
    {
        // failed to track entire leg
        return 0.0;
    }

    return bodyLength + legLength + head_divergence;
}


/**********************************
* This function is called from drawSkeleton every 500 ms if a joint is selected
* 
**********************************/
double CvNect::calculateSpeedOfSelectedJoint()
{
    double speed = 0.0;

    /*cvLine(skeleton,pt[NUI_SKELETON_POSITION_HEAD],pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],CV_RGB(0,255,0));
    cvCircle( skeleton , pt[j] , 5 , m_color_skeletonColorJointSelected , -1 );*/

    speed = length( m_pointCurrent , m_pointNext );

    cout << "length between points " << speed << endl;

    

    return speed;
}

/**********************************
* Using last 30 frame points for the selected joint, draw a poly line tracing the movement
* 
**********************************/
void CvNect::drawPolyLineAtJointPositions()
{
    double jointTravelLength = 0.0;

    for( int i = 0; i < m_jointPositionLast30FrameCount - 1; i++ )
    {
        cvLine( skeleton , m_jointPositionsLast30PointArray[ i ] , m_jointPositionsLast30PointArray[ i + 1 ] , CV_RGB( 255 , 255 , 255 ) );
        jointTravelLength += length( m_jointPositionsLast30Frames[ i ] , m_jointPositionsLast30Frames[ i + 1 ] );
    }
    m_jointSpeed = jointTravelLength;
}

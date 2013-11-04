
#include "stdafx.h"
#include "cvnect.h"

using namespace cv;
using namespace std;

CvNect::CvNect()
{
    
}

CvNect::~CvNect()
{
    
}

HRESULT	CvNect::CvNect_Init()
{
   
    color       = cvCreateImageHeader( cvSize( COLOR_WIDTH , COLOR_HEIGHT )         , IPL_DEPTH_8U , 4       );
    depth       = cvCreateImageHeader( cvSize( DEPTH_WIDTH , DEPTH_HEIGHT )         , IPL_DEPTH_8U , CHANNEL );

    cvNamedWindow( "RGB"	, CV_WINDOW_AUTOSIZE );
    cvNamedWindow( "Depth"	, CV_WINDOW_AUTOSIZE );

	HRESULT hr = NuiInitialize( NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH );

    if( hr != S_OK )
    {
        cout << "NuiInitialize failed " << hr << endl;

        return hr;
    }
    else
    {
        cout << "NuiInitialize successful..\nOpening streams..." << endl;
    }

    cout << "Image stream opening..";

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
    }

    cout << "Depth stream opening..";

    m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    hr = NuiImageStreamOpen( 
		NUI_IMAGE_TYPE_DEPTH ,
        NUI_IMAGE_RESOLUTION_640x480 ,
        0 ,
        2 ,
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

    m_fpsColor      = 0;
    m_fpsDepth      = 0;

    m_hEvWaitColorWrite = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hEvWaitDepthWrite = CreateEvent( NULL , FALSE , FALSE , NULL );

    // Start the Nui processing thread
    m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hThNuiProcess     = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );	

    return hr;
}

void	CvNect::CvNect_UnInit()
{

    // Stop Nui processing thread
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

	// clean image data
    cvReleaseImageHeader( &color );
	delete color;
	color = NULL;
    cvReleaseImageHeader( &depth );
	delete depth;
	depth = NULL;

	// release writers
	if( m_vWriterColor.isOpened() )
    {
		m_vWriterColor.release();
    }
	if( m_vWriterDepth.isOpened() )
    {
        m_vWriterDepth.release();
    }

	// close openCV windows
    cvDestroyWindow("RGB");
    cvDestroyWindow( "Depth" );

	// shut down NUI interface
    NuiShutdown();
}

void	CvNect::CvNect_Zero()
{
    m_hThNuiProcess             = NULL;
    m_hEvNuiProcessStop         = NULL;

    m_hEvWaitColorWrite         = NULL;
    m_hEvWaitDepthWrite         = NULL;

    m_hNextColorFrameEvent      = NULL;
    m_hNextDepthFrameEvent      = NULL;

    m_pVideoStreamHandle        = NULL;
    m_pDepthStreamHandle        = NULL;

    m_recordColor				= false;
    m_recordDepth               = false;

    m_ColorFramesTotal          = 0;
    m_LastColorFramesTotal      = 0;

    m_DepthFramesTotal          = 0;
    m_LastDepthFramesTotal      = 0;
}

string CvNect::StartColorRecording( int fileNumber )
{
    stringstream fileName;
    
    fileName << "colorout" << fileNumber << ".avi";

    const string colorFileName = fileName.str();

	if( !m_vWriterColor.isOpened() )
    {
		if( m_vWriterColor.open( colorFileName , CV_FOURCC_DEFAULT , 30 , cvSize( COLOR_WIDTH , COLOR_HEIGHT ) ) )
        {
            m_recordColor = true;
        }
        else
        {
            return "Failed to open vWriterColor!";
        }
    }
    else
    {
        return "vWriterColor is already open!";
    }

    stringstream outputStream;

    outputStream << "Recording color to " << colorFileName;

    string output = outputStream.str();

    return output;
}

bool CvNect::StopColorRecording()
{
    if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hEvWaitColorWrite, INFINITE ) )
    {
		if( m_vWriterColor.isOpened() )
        {
            m_recordColor = false;

            m_vWriterColor.release();

            return 0;
        }
    }

    return 1;
}

string CvNect::StartDepthRecording( int fileNumber )
{
    stringstream fileName;

    fileName << "depthout" << fileNumber << ".avi";

    string depthFileName = fileName.str();

	if( !m_vWriterDepth.isOpened() )
    {
        if( m_vWriterDepth.open( depthFileName , CV_FOURCC_DEFAULT , 30 , cvSize( DEPTH_WIDTH , DEPTH_HEIGHT ) ) )
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

bool CvNect::StopDepthRecording()
{
    if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hEvWaitDepthWrite , INFINITE ) )
    {
        if( m_vWriterDepth.isOpened() )
        {
            m_recordDepth = false;

            m_vWriterDepth.release();

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
	HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop , m_hNextDepthFrameEvent, m_hNextColorFrameEvent };
    int    nEventIdx;

    cout << "Nui_ProcessThread starting main loop.." << endl;

    // Main thread loop
    bool continueProcessing = true;
    while ( continueProcessing )
    {
        // Wait for any of the events to be signalled
        nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, INFINITE );

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

        if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hNextColorFrameEvent, 30 ) )
        {
            //cout << "got color frame" << endl;
            if ( drawColor() == 0 )
            {
                ++m_ColorFramesTotal;
            }
        }
    }

	cout << "Returned safely from nui process thread!" << endl;

    return 0;
}

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

		if( m_vWriterColor.isOpened() && m_recordColor )
        {
			Mat iplimg( color , true );

			m_vWriterColor.write( color );
        }

        SetEvent( m_hEvWaitColorWrite );
    }

    cvShowImage("RGB",color);

    NuiImageStreamReleaseFrame( m_pVideoStreamHandle , pImageFrame );

    return 0;
}

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

		if( m_vWriterDepth.isOpened() && m_recordDepth == true )
        {
            Mat iplimg( depth , true );

            m_vWriterDepth.write( depth );
        }

        SetEvent( m_hEvWaitDepthWrite );

    }

	cvShowImage("Depth",depth);

    NuiImageStreamReleaseFrame( m_pDepthStreamHandle , pImageFrame );

    return 0;
}

#include <iostream>

#include "cvnect.h"

using namespace std;

//DWORD WINAPI mainThread( LPVOID pParam )
//{
//    CvNect  *CvNectApp = new CvNect();
//
//    CvNectApp -> CvNect_Zero();
//
//    HRESULT hr = CvNectApp -> CvNect_Init();
//
//    if( hr == S_OK )
//    {
//        while( 1 )
//        {
//            int c = cvWaitKey( 0 );
//
//            if( c == 27 || c == 'q' || c == 'Q' )
//            {
//                CvNectApp -> CvNect_UnInit();
//
//                break;
//            }
//        }
//    }
//    else
//    {
//        cout << "Failed to initialize CvNect." << endl;
//    }
//
//    if( CvNectApp )
//    {
//        delete CvNectApp;
//    }
//
//    return 0;
//}

void resetElevation()
{
    LONG	angle;
    HRESULT hr;

    cout << "Checking elevation angle.." << endl;

    hr = NuiCameraElevationGetAngle( &angle );

    if( SUCCEEDED( hr ) )
    {
        if( angle != 0 )
        {
            cout << "Elevation angle is not 0" << " angle: " << angle << endl;

            cout << "Setting angle to 0.." << endl;

            angle = 0;

            hr = NuiCameraElevationSetAngle( angle );

            if( SUCCEEDED( hr ) )
            {
                cout << "Success.." << endl;
            }
            else
            {
                cout << "Failed to set elevation angle to 0.." << endl;
            }
        }
        else
        {
            cout << "Elevation angle is " << angle << endl;
        }
    }
    else
    {
        cout << "Failed to set elevation angle.." << endl;
    }

}

int main( int argc , char **argv )
{
    cout << "Starting application.." << endl;

    bool /*toggleColorRecord      = false ,*/
        toggleDepthRecord      = false ,
        toggleSkeletonRecord   = false ,
        toggleSeatedMode       = false;

    int  /*colorFileNumber    = 0 ,*/
        depthFileNumber    = 0 ,
        skeletonFileNumber = 0;

    HRESULT hr;

    LONG	elevationAngle = 0;

    CvNect CvNectApp;

    CvNectApp.CvNect_Zero();

    hr = CvNectApp.CvNect_Init();

    if( SUCCEEDED( hr ) )
    {

        //resetElevation();

        while( 1 )
        {

            Sleep( 1000 );

            int c = cvWaitKey( 0 );

            if( c != -1 )
            {
                cout << "Key pressed: " << c << endl;
            }

            switch( c )
            {

                /*if( c == 'c' || c == 'C' || c == 'b' || c == 'B' )
                {
                if( !toggleColorRecord )
                {
                cout << CvNectApp.StartColorRecording( colorFileNumber++ ) << endl;
                toggleColorRecord = true;
                }
                else
                {
                if( CvNectApp.StopColorRecording() == 0 )
                {
                cout << "Stopped recording color." << endl;
                toggleColorRecord = false;
                }
                else
                {
                cout << "Can not find active VideoWriter for color." << endl;
                toggleColorRecord = false;
                }
                }
                }*/

            case 'd'	:
            case 'D'	:
            case 'b'	:
            case 'B'	:


                if( !toggleDepthRecord )
                {
                    cout << CvNectApp.StartDepthRecording( depthFileNumber++ ) << endl;
                    toggleDepthRecord = true;
                }
                else
                {
                    if( CvNectApp.StopDepthRecording() == 0 )
                    {
                        cout << "Stopped recording depth." << endl;
                        toggleDepthRecord = false;
                    }
                    else
                    {
                        cout << "Can not find active VideoWriter for depth." << endl;
                        toggleDepthRecord = false;
                    }
                }

                break;


                /*if( c == 's' || c == 'S' || c == 'b' || c == 'B' )
                {
                if( !toggleSkeletonRecord )
                {
                cout << CvNectApp.StartSkeletonRecording( skeletonFileNumber++ ) << endl;
                toggleSkeletonRecord = true;
                }
                else
                {
                if( CvNectApp.StopSkeletonRecording() == 0 )
                {
                cout << "Stopped recording skeleton." << endl;
                toggleSkeletonRecord = false;
                }
                else
                {
                cout << "Can not find active VideoWriter for skeleton." << endl;
                toggleSkeletonRecord = false;
                }
                }
                }*/

            case 32				:

                resetElevation();

                break;

            case 2490368		:

                elevationAngle += 3;

                if( elevationAngle <= 27 )
                {
                    if( NuiCameraElevationSetAngle( elevationAngle ) == S_OK )
                    {
                        cout << "Camera elevation ++ 3 degress.." << elevationAngle << endl;
                    }
                    else
                    {
                        cout << "Failed to set camera elevation.." << endl;
                    }
                }
                else
                {
                    cout << "Camera angle is too high: " << elevationAngle << endl;
                }

                break;

            case 2621440		:

                elevationAngle -= 3;

                if( elevationAngle >= -27 )
                {
                    if( NuiCameraElevationSetAngle( elevationAngle ) == S_OK )
                    {
                        cout << "Camera elevation -- 3 degress.." << elevationAngle << endl;
                    }
                    else
                    {
                        cout << "Failed to set camera elevation.." << endl;
                    }
                }
                else
                {
                    cout << "Camera angle is too low: " << elevationAngle << endl;
                }

                break;

            case 's'    :
            case 'S'    :

                if( !toggleSeatedMode )
                {
                    cout << "Changing skeleton tracking mode to seated..." << endl;

                    toggleSeatedMode = true;
                }
                else
                {
                    cout << "Changing skeleton tracking mode to default..." << endl;

                    toggleSeatedMode = false;
                }

                CvNectApp.UpdateSkeletonTrackingSeatedModeFlag( NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT , toggleSeatedMode );

                /*NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT,
        (mode == SV_TRACKING_MODE_SEATED) );*/
                break;

            case 27		:
            case 'q'	:
            case 'Q'	:

                CvNectApp.CvNect_UnInit();



                return 0;

            }
        }
    }
    else
    {
        cout << "Failed to initialize CvNect." << endl;
    }

    /*if( CvNectApp )
    {
    delete CvNectApp;
    }*/

    /*HANDLE mainThreadHandle;

    mainThreadHandle = CreateThread( NULL , 0 , mainThread , NULL , 0 , NULL );

    while( 1 )
    {
    Sleep( 1000 );
    }

    return 0;*/

    return 0;
}
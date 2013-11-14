
#include "stdafx.h"

#include "cvnect.h"

using namespace std;

/// <summary>
/// Resets the camera angle to 0 
/// </summary>
inline void resetElevation()
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

/// <summary>
/// Application entry point 
/// </summary>
int main( int argc , char **argv )
{
    cout << "Starting application.." << endl;

    bool followRecordStarted = false;

    HRESULT hr;

    LONG elevationAngle = 0;

    CvNect CvNectInstance;

    CvNectInstance.CvNect_Zero();

    hr = CvNectInstance.CvNect_Init();

	bool	toggleColorRecord	= false;
    bool    toggleDepthRecord	= false;
    int		colorFileNumber		= 0;
    int		depthFileNumber		= 0;

    if( hr == S_OK )
    {
        while( 1 )
        {
			Sleep( 1000 );

			int c = waitKey(0);

			switch( c )
			{
				case 32	:	// space bar

					resetElevation();
					break;

				case 27:
				case 'q':
				case 'Q':	

					CvNectInstance.CvNect_UnInit();
					break;

				case 'd'	:
				case 'D'	:

					if( !toggleDepthRecord )
					{
						cout << CvNectInstance.StartDepthRecording( depthFileNumber++ ) << endl;
						toggleDepthRecord = true;
					}
					else
					{
						if( CvNectInstance.StopDepthRecording() == 0 )
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

				case 'c'	:
				case 'C'	:

					if( !toggleColorRecord )
					{
						cout << CvNectInstance.StartColorRecording( colorFileNumber++ ) << endl;
						toggleColorRecord = true;
					}
					else
					{
						if( CvNectInstance.StopColorRecording() == 0 )
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

					break;

				case 'b'	:
				case 'B'	:

					if( !toggleColorRecord && !toggleDepthRecord )
					{
						cout << CvNectInstance.StartColorRecording( colorFileNumber++ ) << endl;
						cout << CvNectInstance.StartDepthRecording( depthFileNumber++ ) << endl;
						toggleColorRecord = true;
						toggleDepthRecord = true;
					}
					else
					{
						if( CvNectInstance.StopColorRecording() == 0 && CvNectInstance.StopDepthRecording() == 0 )
						{
							cout << "Stopped recording color and depth." << endl;
							toggleColorRecord = false;
							toggleDepthRecord = false;
						}
						else
						{
							cout << "Can not find active VideoWriter for color and depth." << endl;
							toggleColorRecord = false;
							toggleDepthRecord = false;
						}
					}

					break;

				default:

					break;
			};         
        }
    }
    else
    {
        cout << "Failed to initialize CvNect." << endl;
    }

    return 0;
}
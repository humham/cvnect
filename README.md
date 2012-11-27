======
cvnect v1.271112
======

a project using microsoft kinect sdk and opencv

======
changelog v1.271112
======

- CvNect::UpdateSkeletonTrackingSeatedModeFlag function added for toggling seated skeleton tracking mode on/off. (activated from using keyboard by pressing 's')
- CvNect::length2D , magnitude1D , CvNect::detectDirectionOfSelectedJointMotion  , CvNect::drawDirectedJointMotion functions added for detecting direction of movement of the selected joint and an arrow representing direction of movement vector with size and color changing as a subject of speed added.

======
Compile
======

- Current version of CvNect requires Microsoft Kinect SDK 1.6 and OpenCV 2.4.2 installed on the system to compile
- Links to the required libraries:
  -- Kinect SDK 1.6 http://www.microsoft.com/en-us/kinectforwindows/develop/developer-downloads.aspx
  -- OpenCV 2.4.2	http://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.4.2/
- Make sure environment variables are set for sdk and opencv directories or adjust project settings accordingly
  -- "..\Microsoft SDKs\Kinect\v1.6" -> "$(KINECTSDK10_DIR)"
  -- "..\opencv\build" -> "$(OPENCV_DIR)"
- Current repository includes a project file for Microsoft Visual Studio 2010

======
Run
======

- .NET Framework 4.0 or later required to run -- http://msdn.microsoft.com/en-us/vstudio/aa496123
- If you don't have the SDK installed and just want to run the project, Kinect for Windows Runtime v1.6 
  needs to be installed and Kinect must be plugged to your pc before running CvNect.exe in order to initialize NUI Api. 
  -- Kinect for Windows Runtime v1.6 http://go.microsoft.com/fwlink/?LinkId=265291
  
======
Support
======

- If you have any questions/problems regarding CvNect feel free to contact me:

  Kerem Gocen - keremgocen@gmail.com
  


#include "JointLogger.h"

/// <summary>
/// Default constructor
/// </summary>
JointLogger::JointLogger()
{
  cout << "JointLogger const called!" << endl;
}

/// <summary>
/// Constructor
/// </summary>
JointLogger::JointLogger( NUI_SKELETON_POSITION_INDEX selected_index )
{
    selected_sekeleton_index = selected_index;

    outputFileCount = 0;
    outputFileSize  = 0;
}

/// <summary>
/// Destructor
/// </summary>
JointLogger::~JointLogger()
{
    if( out.is_open() )
    {
        out.close();
    }
}


/// <summary>
/// Initialize
/// </summary>
void    JointLogger::JointLogger_Init( string jointName , NUI_SKELETON_POSITION_INDEX selected_index )
{
    cout << "JointLogger JointLogger_Init called!" << endl;

    selected_sekeleton_index = selected_index;

    jointLoggerFileName = (unsigned char *)malloc(sizeof( MAX_PATH ));

    if( !jointName.empty() )
     {
         fileName = jointName;

         cout << "File name is " << fileName << endl;
     }

    if( CreateNewJointLog() )
    {
        cout << "Log created!" << endl;
    }
    else
    {
        cout << "ERROR:Failed to create log!" << endl;
    }

}


/// <summary>
/// Uninitialize
/// </summary>
void    JointLogger::JointLogger_UnInit()
{
}


/// <summary>
/// Insert data into log for the selected joint
/// </summary>
void    JointLogger::LogData( DWORD timeStamp , bool *selectedJoints , CvPoint *positions )
{
    if( outputFileSize == MAX_OUTPUT_FILE_SIZE )
    {
        CreateNewJointLog();
    }

    if( out.is_open() )
    {
        for( int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++ )
        {
            //cout << "JointLogger selectedSkeletonJoint " << i << " " << *(selectedJoints+i) << endl;
            if( *(selectedJoints+i) == true )
            {
                //cout << "JointLogger LogData writing! x:" << (positions+i)->x << " y:" << (positions+i)->y << endl;
                out << endl << "jointIndex:" << i << " x:" << (positions+i)->x << " y:" << (positions+i)->y << " timestamp:" << timeStamp;
                outputFileSize += sizeof( CvPoint );
            }
        }
    }
}


 bool JointLogger::CreateNewJointLog()
 {
     cout << "JointLogger CreateNewJointLog called!" << endl;

     if( outputFileCount == MAX_OUTPUT_FILE_COUNT )
     {
         outputFileCount = 0;
     }

     string temp = fileName + std::to_string( outputFileCount ) + ".txt";

     // check if there is an existing open file
     if( out.is_open() )
     {
         out << endl << "***END OF FILE***";

         out.close();

         outputFileSize = 0;

         cout << "Closing previous file" << endl;
     }

     if( !fileName.empty() )
     {
         out.open( temp );

         if( out.is_open() )
         {
             out << "***START OF FILE***";

             outputFileCount++;

             cout << "Opened new file! filecount:" << outputFileCount << endl;

             return TRUE;
         }
         else
         {
             cout << "ERROR:Failed to open file" << endl;
         }

         
     }
     else
     {
         cout << "ERROR:Empty file name" << endl;
     }

     
    
     return FALSE;
 }
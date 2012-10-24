#ifndef _JOINTLOGGER_H_
#define _JOINTLOGGER_H_

#include "stdafx.h"

#include <Windows.h>
#include <fstream>
#include <cstdlib>

#include "common.h"

using namespace std;

class JointLogger
{

    static const unsigned int       MAX_OUTPUT_FILE_SIZE = 1024*10;  // 10 MB // 20 * 1024 * 1024 = 20 Mb log file limit
    static const unsigned int       MAX_OUTPUT_FILE_COUNT = 10;

public:

    /// <summary>
    /// Dafault constructor
    /// </summary>
    JointLogger();

    /// <summary>
    /// Constructor
    /// </summary>
    JointLogger( NUI_SKELETON_POSITION_INDEX selected_index );

    /// <summary>
    /// Destructor
    /// </summary>
    ~JointLogger();

    /// <summary>
    /// Initialize
    /// </summary>
    void    JointLogger_Init( string jointName , NUI_SKELETON_POSITION_INDEX selected_index );

    /// <summary>
    /// Uninitialize
    /// </summary>
    void    JointLogger_UnInit();

    /// <summary>
    /// Insert data into log for the selected joint
    /// </summary>
    void LogData( DWORD timeStamp , bool *selectedJoints , CvPoint *positions );

private:

    unsigned char   *jointLoggerFileName;
    string   fileName;

    _ULonglong  outputFileCount;
    UINT32  outputFileSize;

    ofstream out;

    NUI_SKELETON_POSITION_INDEX selected_sekeleton_index;

    bool CreateNewJointLog( );

};

#endif /* _JOINTLOGGER_H_ */
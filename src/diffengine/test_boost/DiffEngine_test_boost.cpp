/* Unit test for Diff using BOOST framework                          */
/*-------------------------------------------------------------------*/
/* Main class for testing base behavior of the DiffEngine            */
/*                                                                   */
/* Uwe Rosner                                                        */
/* 09/2018                                                           */
/*-------------------------------------------------------------------*/

#define BOOST_TEST_MAIN
#if !defined( WIN32 )
    #define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>

#include <list>
#include <string>
#include <vector>

#include "DiffInputFileLinux.h"
#include "DiffOutputFileLinux.h"
#include "DiffEngine.h"
#include "DiffLine.h"

#include "DiffWindowTextAreaMock.h"
#include "ProgressReporter.h"

#include "SelectableDiffFile.h"
#include "StaticSelection.h"
#include "DynamicSelection.h"
#include "SelectionBase.h"
#include "TextSelectionLine.h"

#include "DiffFileSearchEngine.h"
#include "DiffFileSearchEngineSteadily.h"
#include "DiffFileSearchResult.h"

ProgressReporter progress;

void printFile(DiffFileBase& file)
{
    const char* pStates[] = {"[   ]", "[ADD]", "[CHN]", "[DEL]", "[???]"};
    for(size_t i = 0; i < file.getNumLines(); i++)
    {
      const char* pLineState;
      DiffLine* pLine = file[i];
      switch (pLine->getState())
      {
      case DiffLine::Normal:
        pLineState = pStates[0];
        break;

      case DiffLine::Added:
        pLineState = pStates[1];
        break;

      case DiffLine::Changed:
        pLineState = pStates[2];
        break;

      case DiffLine::Deleted:
        pLineState = pStates[3];
        break;
      
      default:
        pLineState = pStates[4];
        break;
      }

      printf("%s: %s %s\n", pLine->getLineNumText(), pLineState, pLine->getText());
    }
}


/**
 * testcase_02
 *
 *   Left.txt     |   Right.txt
 *   ------------------------------
 *   AAAA         |   AAAA
 *   BBBB         |   BBBB
 *   CCCC         |   1111
 *   DDDD         |   2222
 *   EEEE         |   CCCC
 *   FFFF         |   DDDD
 *   GGGG         |   EEEE
 *                |   FFFF
 *                |   GGGG
 *
 */
BOOST_AUTO_TEST_CASE( testcase_02 )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_02_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_02_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 10);
    BOOST_CHECK_EQUAL(diffA[0]->getText(), "AAAA");
    BOOST_CHECK_EQUAL(diffA[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[1]->getText(), "BBBB");
    BOOST_CHECK_EQUAL(diffA[1]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[2]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[2]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[3]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[3]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[4]->getText(), "CCCC");
    BOOST_CHECK_EQUAL(diffA[4]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[5]->getText(), "DDDD");
    BOOST_CHECK_EQUAL(diffA[5]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[6]->getText(), "EEEE");
    BOOST_CHECK_EQUAL(diffA[6]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[7]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[7]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[8]->getText(), "FFFF");
    BOOST_CHECK_EQUAL(diffA[8]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[9]->getText(), "GGGG");
    BOOST_CHECK_EQUAL(diffA[9]->getState(), DiffLine::Normal);

    BOOST_CHECK_EQUAL(diffB.getNumLines(), 10);
    BOOST_CHECK_EQUAL(diffB[0]->getText(), "AAAA");
    BOOST_CHECK_EQUAL(diffB[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[1]->getText(), "BBBB");
    BOOST_CHECK_EQUAL(diffB[1]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[2]->getText(), "1111");
    BOOST_CHECK_EQUAL(diffB[2]->getState(), DiffLine::Added);
    BOOST_CHECK_EQUAL(diffB[3]->getText(), "2222");
    BOOST_CHECK_EQUAL(diffB[3]->getState(), DiffLine::Added);
    BOOST_CHECK_EQUAL(diffB[4]->getText(), "CCCC");
    BOOST_CHECK_EQUAL(diffB[4]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[5]->getText(), "DDDD");
    BOOST_CHECK_EQUAL(diffB[5]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[6]->getText(), "EEEE");
    BOOST_CHECK_EQUAL(diffB[6]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[7]->getText(), "3333");
    BOOST_CHECK_EQUAL(diffB[7]->getState(), DiffLine::Added);
    BOOST_CHECK_EQUAL(diffB[8]->getText(), "FFFF");
    BOOST_CHECK_EQUAL(diffB[8]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[9]->getText(), "GGGG");
    BOOST_CHECK_EQUAL(diffB[9]->getState(), DiffLine::Normal);

    // This test has 2 difference blocks of lines added to the right side
    BOOST_CHECK_EQUAL(diffEngine.getNumAdded(), 2);
    BOOST_CHECK_EQUAL(m_DiffIndices.size(), 2);

    // Now verify that the two difference blocks start at the line 
    // indexes 2 and 7
    std::list<size_t>::iterator it = m_DiffIndices.begin();
    BOOST_CHECK_EQUAL((*it++), 2);
    BOOST_CHECK_EQUAL((*it), 7);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


/**
 * testcase_03_FB101-02-Simple
 *
 *   Left.txt     |   Right.txt
 *   ------------------------------
 *   AAAA         |   AAAA
 *   BBBB         |   FFFF
 *   CCCC         |   CCCC
 *   DDDD         |   DDDD
 *   EEEE         |   EEEE
 *   FFFF         |   FFFF
 *   GGGG         |   GGGG
 *                |
 *
 *   >> Changed line 2
 */
BOOST_AUTO_TEST_CASE( testcase_03_simple )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_03_FB101-02-Simple_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_03_FB101-02-Simple_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 7);
    BOOST_CHECK_EQUAL(diffA[0]->getText(), "AAAA");
    BOOST_CHECK_EQUAL(diffA[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[0]->getLineNumText(), "1 ");
    BOOST_CHECK_EQUAL(diffA[1]->getText(), "BBBB");
    BOOST_CHECK_EQUAL(diffA[1]->getState(), DiffLine::Changed);
    BOOST_CHECK_EQUAL(diffA[1]->getLineNumText(), "2 ");
    BOOST_CHECK_EQUAL(diffA[2]->getText(), "CCCC");
    BOOST_CHECK_EQUAL(diffA[2]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[2]->getLineNumText(), "3 ");
    BOOST_CHECK_EQUAL(diffA[3]->getText(), "DDDD");
    BOOST_CHECK_EQUAL(diffA[3]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[3]->getLineNumText(), "4 ");
    BOOST_CHECK_EQUAL(diffA[4]->getText(), "EEEE");
    BOOST_CHECK_EQUAL(diffA[4]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[4]->getLineNumText(), "5 ");
    BOOST_CHECK_EQUAL(diffA[5]->getText(), "FFFF");
    BOOST_CHECK_EQUAL(diffA[5]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[5]->getLineNumText(), "6 ");
    BOOST_CHECK_EQUAL(diffA[6]->getText(), "GGGG");
    BOOST_CHECK_EQUAL(diffA[6]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[6]->getLineNumText(), "7 ");



    BOOST_CHECK_EQUAL(diffB.getNumLines(), 7);
    BOOST_CHECK_EQUAL(diffB[0]->getText(), "AAAA");
    BOOST_CHECK_EQUAL(diffB[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[0]->getLineNumText(), "1 ");
    BOOST_CHECK_EQUAL(diffB[1]->getText(), "FFFF");
    BOOST_CHECK_EQUAL(diffB[1]->getState(), DiffLine::Changed);
    BOOST_CHECK_EQUAL(diffB[1]->getLineNumText(), "2 ");
    BOOST_CHECK_EQUAL(diffB[2]->getText(), "CCCC");
    BOOST_CHECK_EQUAL(diffB[2]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[2]->getLineNumText(), "3 ");
    BOOST_CHECK_EQUAL(diffB[3]->getText(), "DDDD");
    BOOST_CHECK_EQUAL(diffB[3]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[3]->getLineNumText(), "4 ");
    BOOST_CHECK_EQUAL(diffB[4]->getText(), "EEEE");
    BOOST_CHECK_EQUAL(diffB[4]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[4]->getLineNumText(), "5 ");
    BOOST_CHECK_EQUAL(diffB[5]->getText(), "FFFF");
    BOOST_CHECK_EQUAL(diffB[5]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[5]->getLineNumText(), "6 ");
    BOOST_CHECK_EQUAL(diffB[6]->getText(), "GGGG");
    BOOST_CHECK_EQUAL(diffB[6]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[6]->getLineNumText(), "7 ");
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
  

}



BOOST_AUTO_TEST_CASE( testcase_03_var2 )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_03_var2_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_03_var2_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 8);
    BOOST_CHECK_EQUAL(diffA[0]->getText(), "AAAA");
    BOOST_CHECK_EQUAL(diffA[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[0]->getLineNumText(), "1 ");
    BOOST_CHECK_EQUAL(diffA[1]->getText(), "BBBB");
    BOOST_CHECK_EQUAL(diffA[1]->getState(), DiffLine::Changed);
    BOOST_CHECK_EQUAL(diffA[1]->getLineNumText(), "2 ");
    BOOST_CHECK_EQUAL(diffA[2]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[2]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[2]->getLineNumText(), "  ");
    BOOST_CHECK_EQUAL(diffA[3]->getText(), "CCCC");
    BOOST_CHECK_EQUAL(diffA[3]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[3]->getLineNumText(), "3 ");
    BOOST_CHECK_EQUAL(diffA[4]->getText(), "DDDD");
    BOOST_CHECK_EQUAL(diffA[4]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[4]->getLineNumText(), "4 ");
    BOOST_CHECK_EQUAL(diffA[5]->getText(), "EEEE");
    BOOST_CHECK_EQUAL(diffA[5]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[5]->getLineNumText(), "5 ");
    BOOST_CHECK_EQUAL(diffA[6]->getText(), "FFFF");
    BOOST_CHECK_EQUAL(diffA[6]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[6]->getLineNumText(), "6 ");
    BOOST_CHECK_EQUAL(diffA[7]->getText(), "GGGG");
    BOOST_CHECK_EQUAL(diffA[7]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[7]->getLineNumText(), "7 ");

    BOOST_CHECK_EQUAL(diffB.getNumLines(), 8);
    BOOST_CHECK_EQUAL(diffB[0]->getText(), "AAAA");
    BOOST_CHECK_EQUAL(diffB[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[0]->getLineNumText(), "1 ");
    BOOST_CHECK_EQUAL(diffB[1]->getText(), "FFFF");
    BOOST_CHECK_EQUAL(diffB[1]->getState(), DiffLine::Changed);
    BOOST_CHECK_EQUAL(diffB[1]->getLineNumText(), "2 ");
    BOOST_CHECK_EQUAL(diffB[2]->getText(), "ffff");
    BOOST_CHECK_EQUAL(diffB[2]->getState(), DiffLine::Added);
    BOOST_CHECK_EQUAL(diffB[2]->getLineNumText(), "3 ");
    BOOST_CHECK_EQUAL(diffB[3]->getText(), "CCCC");
    BOOST_CHECK_EQUAL(diffB[3]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[3]->getLineNumText(), "4 ");
    BOOST_CHECK_EQUAL(diffB[4]->getText(), "DDDD");
    BOOST_CHECK_EQUAL(diffB[4]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[4]->getLineNumText(), "5 ");
    BOOST_CHECK_EQUAL(diffB[5]->getText(), "EEEE");
    BOOST_CHECK_EQUAL(diffB[5]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[5]->getLineNumText(), "6 ");
    BOOST_CHECK_EQUAL(diffB[6]->getText(), "FFFF");
    BOOST_CHECK_EQUAL(diffB[6]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[6]->getLineNumText(), "7 ");
    BOOST_CHECK_EQUAL(diffB[7]->getText(), "GGGG");
    BOOST_CHECK_EQUAL(diffB[7]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[7]->getLineNumText(), "8 ");
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


/**
 * testcase_04
 *
 *   Left.txt       |   Right.txt
 *   ------------------------------
 *   Line 1         |   Line 1
 *   Line 2         |   Line 2
 *   Line 3         |
 *   Line 4         |   Line 4
 *                  |
 *
 *   >> Cleared "Line 3" (set to empty) in right file
 */
BOOST_AUTO_TEST_CASE( testcase_04 )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_04_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_04_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 4);
    BOOST_CHECK_EQUAL(diffA[0]->getText(), "Line 1");
    BOOST_CHECK_EQUAL(diffA[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[1]->getText(), "Line 2");
    BOOST_CHECK_EQUAL(diffA[1]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[2]->getText(), "Line 3");
    BOOST_CHECK_EQUAL(diffA[2]->getState(), DiffLine::Changed);
    BOOST_CHECK_EQUAL(diffA[3]->getText(), "Line 4");
    BOOST_CHECK_EQUAL(diffA[3]->getState(), DiffLine::Normal);

    BOOST_CHECK_EQUAL(diffB.getNumLines(), 4);
    BOOST_CHECK_EQUAL(diffB[0]->getText(), "Line 1");
    BOOST_CHECK_EQUAL(diffB[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[1]->getText(), "Line 2");
    BOOST_CHECK_EQUAL(diffB[1]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[2]->getText(), "");
    BOOST_CHECK_EQUAL(diffB[2]->getState(), DiffLine::Changed);
    BOOST_CHECK_EQUAL(diffB[3]->getText(), "Line 4");
    BOOST_CHECK_EQUAL(diffB[3]->getState(), DiffLine::Normal);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


/**
 * testcase_05
 *
 *   Left.txt       |   Right.txt
 *   ------------------------------
 *   Line 1         |   Line 1
 *   Line 3         |   Line 2
 *   Line 4         |   Line 3
 *                  |   Line 4
 *                  |
 *
 *   >> Inserted "Line 2" in right file
 *
 */
BOOST_AUTO_TEST_CASE( testcase_05 )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_05_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_05_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 4);
    BOOST_CHECK_EQUAL(diffA[0]->getText(), "Line 1");
    BOOST_CHECK_EQUAL(diffA[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[1]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[1]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[2]->getText(), "Line 3");
    BOOST_CHECK_EQUAL(diffA[2]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[3]->getText(), "Line 4");
    BOOST_CHECK_EQUAL(diffA[3]->getState(), DiffLine::Normal);

    BOOST_CHECK_EQUAL(diffB.getNumLines(), 4);
    BOOST_CHECK_EQUAL(diffB[0]->getText(), "Line 1");
    BOOST_CHECK_EQUAL(diffB[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[1]->getText(), "Line 2");
    BOOST_CHECK_EQUAL(diffB[1]->getState(), DiffLine::Added);
    BOOST_CHECK_EQUAL(diffB[2]->getText(), "Line 3");
    BOOST_CHECK_EQUAL(diffB[2]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[3]->getText(), "Line 4");
    BOOST_CHECK_EQUAL(diffB[3]->getState(), DiffLine::Normal);

  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


/**
 * testcase_06
 *
 *   Left.txt       |   Right.txt
 *   ------------------------------
 *   Line 1         |   Line 1
 *   Line 4         |   Line 2
 *   Line 5         |   Line 3
 *   Line 6         |   Line 4
 *   Line 7         |   Line 5
 *   Line 8         |   Line 8
 *   ab             |   Line 9
 *   Line 9         |   cd
 *   Line 10        |   Line 10
 *
 */
BOOST_AUTO_TEST_CASE( DiffTest_06_Mixed )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_06_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_06_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 12);
    BOOST_CHECK_EQUAL(diffA[0]->getText(), "Line 1");
    BOOST_CHECK_EQUAL(diffA[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[0]->getLineNumText(), "1 ");
    BOOST_CHECK_EQUAL(diffA[1]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[1]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[1]->getLineNumText(), "  ");
    BOOST_CHECK_EQUAL(diffA[2]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[2]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[2]->getLineNumText(), "  ");
    BOOST_CHECK_EQUAL(diffA[3]->getText(), "Line 4");
    BOOST_CHECK_EQUAL(diffA[3]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[3]->getLineNumText(), "2 ");
    BOOST_CHECK_EQUAL(diffA[4]->getText(), "Line 5");
    BOOST_CHECK_EQUAL(diffA[4]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[4]->getLineNumText(), "3 ");
    BOOST_CHECK_EQUAL(diffA[5]->getText(), "Line 6");
    BOOST_CHECK_EQUAL(diffA[5]->getState(), DiffLine::Deleted);
    BOOST_CHECK_EQUAL(diffA[5]->getLineNumText(), "4 ");
    BOOST_CHECK_EQUAL(diffA[6]->getText(), "Line 7");
    BOOST_CHECK_EQUAL(diffA[6]->getState(), DiffLine::Deleted);
    BOOST_CHECK_EQUAL(diffA[6]->getLineNumText(), "5 ");
    BOOST_CHECK_EQUAL(diffA[7]->getText(), "Line 8");
    BOOST_CHECK_EQUAL(diffA[7]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[7]->getLineNumText(), "6 ");
    BOOST_CHECK_EQUAL(diffA[8]->getText(), "ab");
    BOOST_CHECK_EQUAL(diffA[8]->getState(), DiffLine::Deleted);
    BOOST_CHECK_EQUAL(diffA[8]->getLineNumText(), "7 ");
    BOOST_CHECK_EQUAL(diffA[9]->getText(), "Line 9");
    BOOST_CHECK_EQUAL(diffA[9]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[9]->getLineNumText(), "8 ");
    BOOST_CHECK_EQUAL(diffA[10]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[10]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[10]->getLineNumText(), "  ");
    BOOST_CHECK_EQUAL(diffA[11]->getText(), "Line 10");
    BOOST_CHECK_EQUAL(diffA[11]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[11]->getLineNumText(), "9 ");

    BOOST_CHECK_EQUAL(diffB.getNumLines(), 12);
    BOOST_CHECK_EQUAL(diffB[0]->getText(), "Line 1");
    BOOST_CHECK_EQUAL(diffB[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[0]->getLineNumText(), "1 ");
    BOOST_CHECK_EQUAL(diffB[1]->getLineNumText(), "2 ");
    BOOST_CHECK_EQUAL(diffB[1]->getText(), "Line 2");
    BOOST_CHECK_EQUAL(diffB[1]->getState(), DiffLine::Added);
    BOOST_CHECK_EQUAL(diffB[2]->getText(), "Line 3");
    BOOST_CHECK_EQUAL(diffB[2]->getState(), DiffLine::Added);
    BOOST_CHECK_EQUAL(diffB[2]->getLineNumText(), "3 ");
    BOOST_CHECK_EQUAL(diffB[3]->getText(), "Line 4");
    BOOST_CHECK_EQUAL(diffB[3]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[3]->getLineNumText(), "4 ");
    BOOST_CHECK_EQUAL(diffB[4]->getText(), "Line 5");
    BOOST_CHECK_EQUAL(diffB[4]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[4]->getLineNumText(), "5 ");
    BOOST_CHECK_EQUAL(diffB[5]->getText(), "");
    BOOST_CHECK_EQUAL(diffB[5]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[5]->getLineNumText(), "  ");
    BOOST_CHECK_EQUAL(diffB[6]->getText(), "");
    BOOST_CHECK_EQUAL(diffB[6]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[6]->getLineNumText(), "  ");
    BOOST_CHECK_EQUAL(diffB[7]->getText(), "Line 8");
    BOOST_CHECK_EQUAL(diffB[7]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[7]->getLineNumText(), "6 ");
    BOOST_CHECK_EQUAL(diffB[8]->getText(), "");
    BOOST_CHECK_EQUAL(diffB[8]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[8]->getLineNumText(), "  ");
    BOOST_CHECK_EQUAL(diffB[9]->getText(), "Line 9");
    BOOST_CHECK_EQUAL(diffB[9]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[9]->getLineNumText(), "7 ");
    BOOST_CHECK_EQUAL(diffB[10]->getText(), "cd");
    BOOST_CHECK_EQUAL(diffB[10]->getState(), DiffLine::Added);
    BOOST_CHECK_EQUAL(diffB[10]->getLineNumText(), "8 ");
    BOOST_CHECK_EQUAL(diffB[11]->getText(), "Line 10");
    BOOST_CHECK_EQUAL(diffB[11]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[11]->getLineNumText(), "9 ");

  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


/**
 * testcase_12_endless_loop
 *
 *   Left.txt       |   Right.txt
 *   ------------------------------
 *   Line 1         |   Line 1
 *   Line 2         |   Line 2
 *   Line 3         |
 *                  |
 *   Line 5         |   Line 5
 *                  |
 *
 *   >> Cleared "Line 3" (set to empty) in right file
 *
 * Within this test case the first implementation of the old
 * (not Myers) algorithm from 2003 did hang in an endless loop.
 *
 */
BOOST_AUTO_TEST_CASE( testcase_12_endless_loop )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_12_endless_loop_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_12_endless_loop_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 5);
    BOOST_CHECK_EQUAL(diffA[0]->getText(), "Line 1");
    BOOST_CHECK_EQUAL(diffA[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[1]->getText(), "Line 2");
    BOOST_CHECK_EQUAL(diffA[1]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[2]->getText(), "Line 3");
    BOOST_CHECK_EQUAL(diffA[2]->getState(), DiffLine::Changed);
    BOOST_CHECK_EQUAL(diffA[3]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[3]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[4]->getText(), "Line 5");
    BOOST_CHECK_EQUAL(diffA[4]->getState(), DiffLine::Normal);


    BOOST_CHECK_EQUAL(diffB.getNumLines(), 5);
    BOOST_CHECK_EQUAL(diffB[0]->getText(), "Line 1");
    BOOST_CHECK_EQUAL(diffB[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[1]->getText(), "Line 2");
    BOOST_CHECK_EQUAL(diffB[1]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[2]->getText(), "");
    BOOST_CHECK_EQUAL(diffB[2]->getState(), DiffLine::Changed);
    BOOST_CHECK_EQUAL(diffB[3]->getText(), "");
    BOOST_CHECK_EQUAL(diffB[3]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[4]->getText(), "Line 5");
    BOOST_CHECK_EQUAL(diffB[4]->getState(), DiffLine::Normal);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


/**
 * testcase_12a
 *
 *   Left.txt       |   Right.txt
 *   ------------------------------
 *   Line 1         |   Line 1
 *   Line 2         |   Line 2
 *   Line 3         |
 *                  |
 *   Line 5         |
 *                  |   Line 5
 *                  |
 *
 *   >> Cleared "Line 3" (set to empty) in right file
 *   >> Inserted two empty lines in right file
 *
 *
 */
BOOST_AUTO_TEST_CASE( testcase_12a )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_12a_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_12a_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 6);
    BOOST_CHECK_EQUAL(diffA[0]->getText(), "Line 1");
    BOOST_CHECK_EQUAL(diffA[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[1]->getText(), "Line 2");
    BOOST_CHECK_EQUAL(diffA[1]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[2]->getText(), "Line 3");
    BOOST_CHECK_EQUAL(diffA[2]->getState(), DiffLine::Changed);
    BOOST_CHECK_EQUAL(diffA[3]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[3]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[4]->getText(), "");
    BOOST_CHECK_EQUAL(diffA[4]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffA[5]->getText(), "Line 5");
    BOOST_CHECK_EQUAL(diffA[5]->getState(), DiffLine::Normal);

    BOOST_CHECK_EQUAL(diffB.getNumLines(), 6);
    BOOST_CHECK_EQUAL(diffB[0]->getText(), "Line 1");
    BOOST_CHECK_EQUAL(diffB[0]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[1]->getText(), "Line 2");
    BOOST_CHECK_EQUAL(diffB[1]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[2]->getText(), "");
    BOOST_CHECK_EQUAL(diffB[2]->getState(), DiffLine::Changed);
    BOOST_CHECK_EQUAL(diffB[3]->getText(), "");
    BOOST_CHECK_EQUAL(diffB[3]->getState(), DiffLine::Added);
    BOOST_CHECK_EQUAL(diffB[4]->getText(), "");
    BOOST_CHECK_EQUAL(diffB[4]->getState(), DiffLine::Normal);
    BOOST_CHECK_EQUAL(diffB[5]->getText(), "Line 5");
    BOOST_CHECK_EQUAL(diffB[5]->getState(), DiffLine::Normal);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


size_t getNextDiffLineId(std::list<size_t> m_DiffIndices, 
                         std::list<size_t>::iterator& m_DiffIndicesIterator) 
{
  if(m_DiffIndicesIterator == m_DiffIndices.end())
  {
    // Iterator points to the end. This only is true directly after
    // construction of DiffDocument. Set it to the first item.
    m_DiffIndicesIterator = m_DiffIndices.begin();
  }
  else
  {
    m_DiffIndicesIterator++;

    if(m_DiffIndicesIterator == m_DiffIndices.end())
    {
      // Avoid overflow: back to last valid item
      m_DiffIndicesIterator--;
    }
  }

  return (*m_DiffIndicesIterator);
}

size_t getPrevDiffLineId(std::list<size_t> m_DiffIndices, 
                         std::list<size_t>::iterator& m_DiffIndicesIterator) 
{
  if(m_DiffIndicesIterator == m_DiffIndices.end())
  {
    // Iterator points to the end. This only is true directly after
    // construction of DiffDocument. Set it to the first item.
    m_DiffIndicesIterator = m_DiffIndices.begin();
  }
  else if(m_DiffIndicesIterator != m_DiffIndices.begin())
  {
    // Only if not already the first item
    m_DiffIndicesIterator--;
  }

  return (*m_DiffIndicesIterator);
}

/**
 * testcase_24_1500_numbers
 *
 * 11 changes in two textfiles with about 1500 lines each.
 * Used to test the navigation between diffs.
 */
BOOST_AUTO_TEST_CASE( testcase_24_1500_numbers )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_24_1500_numbers_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_24_1500_numbers_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", 
                          cancelRequested, 
                          m_DiffIndices);
    diffEngine.startCompare();

    std::list<size_t>::iterator m_DiffIndicesIterator = m_DiffIndices.end();

    // Checking the justification of the line numbers
    BOOST_CHECK_EQUAL(diffA[0]->getLineNumText(), "   1 ");
    BOOST_CHECK_EQUAL(diffB[0]->getLineNumText(), "   1 ");
    BOOST_CHECK_EQUAL(diffA[12]->getLineNumText(), "     ");
    BOOST_CHECK_EQUAL(diffB[12]->getLineNumText(), "  13 ");

    long numDiff = diffEngine.getNumDifferences();

    // Testing if the list with the diff start indexes
    // contains 11 'difference blocks' as showed with Kompare
    BOOST_CHECK_EQUAL(m_DiffIndices.size(), 11);
    BOOST_CHECK_EQUAL(m_DiffIndices.size(), numDiff);

    // Testing the 'States' of DiffLines by the indexes in the list.
    // They should be in the order (checked with Linux tool 'Kompare')
    // ADD, CHN, DEL, CHN, CHN, CHN, CHN, ADD, DEL, CHN, CHN
    DiffLine::LineState states[] =
    {
      DiffLine::Added,
      DiffLine::Changed,
      DiffLine::Deleted,
      DiffLine::Changed,
      DiffLine::Changed,
      DiffLine::Changed,
      DiffLine::Changed,
      DiffLine::Added,
      DiffLine::Deleted,
      DiffLine::Changed,
      DiffLine::Changed,
    };

    size_t idx = getNextDiffLineId(m_DiffIndices, m_DiffIndicesIterator);
    int iDiff = 0;

    do
    {
      if(iDiff >= (sizeof states / sizeof states[0]))
      {
        // Don't exceed the array
        break;
      }

      DiffLine::LineState stateNominal = states[iDiff];

      switch(stateNominal)
      {
        case DiffLine::LineState::Deleted:
        {
          DiffLine::LineState stateA = diffA[idx]->getState();
          BOOST_CHECK_EQUAL(stateA, DiffLine::Deleted);
          break;
        }

        case DiffLine::LineState::Added:
        {
          DiffLine::LineState stateB = diffB[idx]->getState();
          BOOST_CHECK_EQUAL(stateB, DiffLine::Added);
          break;
        }

        case DiffLine::LineState::Changed:
        {
          DiffLine::LineState stateA = diffA[idx]->getState();
          DiffLine::LineState stateB = diffB[idx]->getState();
          BOOST_CHECK_EQUAL(stateA, DiffLine::Changed);
          BOOST_CHECK_EQUAL(stateB, DiffLine::Changed);
          break;
        }
      }

      idx = getNextDiffLineId(m_DiffIndices, m_DiffIndicesIterator);
      iDiff++;
    }
    while(idx != 0);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


/**
 * test_TextSelectionItem
 *
 * Test the method TextSelection::getNumMarkedChars().
 * 
 * See Excel sheet 
 *   'ADiffView - selected text and scrolling.xlsx
 * for a visual of this test.
 */
BOOST_AUTO_TEST_CASE( test_TextSelectionBasic )
{
  DiffLine* pLine1 = new DiffLine("");


  std::vector<DiffLine*> textLines;
  textLines.push_back(pLine1);
  try
  {
    StaticSelection selection(textLines);
    
    selection.addBlock(1, 4, 14);
    selection.addBlock(2, 0, 9);
    selection.addBlock(3, 0, 3);

    selection.addBlock(5, 3, 5);
    selection.addBlock(5, 10, 13);

    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(0, 0), 0);
    
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 0), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 1), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 2), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 3), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 4), 11);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 5), 10);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 6), 9);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 13), 2);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 14), 1);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 15), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(1, 16), 0);

    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(2, 0), 10);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(2, 9), 1);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(2, 10), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(2, 11), 0);

    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(3, 0), 4);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(3, 3), 1);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(3, 4), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(3, 5), 0);

    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(4, 0), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(4, 10), 0);

    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 0), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 3), 3);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 4), 2);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 5), 1);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 6), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 7), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 10), 4);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 11), 3);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 12), 2);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 13), 1);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 14), 0);
    BOOST_CHECK_EQUAL(selection.getNumMarkedChars(5, 15), 0);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }

  // Clean up
  std::vector<DiffLine*>::iterator it;
  for(it = textLines.begin(); it != textLines.end(); it++)
  {
    delete *it;
  }

  textLines.clear();
}


/**
 * test_TextSelectionDynamic
 *
 * Test the methods TextSelection::startSelection(..) and
 * TextSelection::updateSelection(..)
 *
 * See Excel sheet
 *   'text_selection_algorithm.xlsx' in other repository Python-Library,
 * directory algo, for a visual of this test.
 */
BOOST_AUTO_TEST_CASE( test_TextSelectionDynamic )
{
  DiffLine* pLine1 = new DiffLine("The first line");
  DiffLine* pLine2 = new DiffLine("The second line");
  DiffLine* pLine3 = new DiffLine("third line");
  DiffLine* pLine4 = new DiffLine("line four");
  DiffLine* pLine5 = new DiffLine("Line no. 5");
  DiffLine* pLine6 = new DiffLine("he she it them our");
  DiffLine* pLine7 = new DiffLine("limit_line_id() tests need this");
  DiffLine* pLine8 = new DiffLine("and this");

  std::vector<DiffLine*> textLines;
  textLines.push_back(pLine1);
  textLines.push_back(pLine2);
  textLines.push_back(pLine3);
  textLines.push_back(pLine4);
  textLines.push_back(pLine5);
  textLines.push_back(pLine6);
  textLines.push_back(pLine7);
  textLines.push_back(pLine8);

  try
  {
    long column;
    unsigned long line;
    std::list<long>::const_iterator it;
    DynamicSelection dynamicSelection(textLines);
    
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(2, 4), 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNextSelectionStart(2, 0), -1);
    
    // 1) Start a dynamicSelection
    dynamicSelection.startSelection(2, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 0);

    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(2, 0), 0);

    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(2, 4), 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNextSelectionStart(2, 0), 4);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 2);

    // 2) Update dynamicSelection to the right
    dynamicSelection.updateSelection(2, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 1);

    column = dynamicSelection.getNextSelectionStart(2, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(2, column), 1);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 2);

    // 3) Then update dynamicSelection to above
    dynamicSelection.updateSelection(1, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 14);

    // Test line 1 (Updated dynamicSelection upward to this);
    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 10);

    // Test line 2 (Selection start);
    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 1);
    it++;
    BOOST_CHECK_EQUAL(*it, 2);

    // 4) Update dynamicSelection to above a 2nd time
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(0, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 28);

    line = 0;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 15);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 0);
    it++;
    BOOST_CHECK_EQUAL(*it, 1);

    // 5) Update dynamicSelection 3 columns to the left
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(0, 2);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 31);

    line = 0;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 2);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 12);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 15);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 0);

    // 6) Update dynamicSelection to one line below
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(1, 2);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 17);

    line = 0;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 2);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 13);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 0);
    it++;
    BOOST_CHECK_EQUAL(*it, 1);

    // 7) Update dynamicSelection to another line below (the start line again);
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(2, 2);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 2);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 2);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 2);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 1);
    it++;
    BOOST_CHECK_EQUAL(*it, 2);

    // 8) Update dynamicSelection to the line below
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(3, 2);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 8);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 6);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 2);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 2);
    it++;
    BOOST_CHECK_EQUAL(*it, 3);

    // 9) Update dynamicSelection to the line below
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(4, 2);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 17);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 6);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 2);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 3);
    it++;
    BOOST_CHECK_EQUAL(*it, 4);

    // 10) Update dynamicSelection 5 columns to the right
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(4, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 22);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 6);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 7);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 4);

    // 11) Update dynamicSelection to the line above
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(3, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 13);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 6);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 7);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 3);
    it++;
    BOOST_CHECK_EQUAL(*it, 4);

    // 12) Update dynamicSelection to the line above (Starting line again);
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(2, 7);
    dynamicSelection.updateSelection(2, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumTotalSelectedChars(), 3);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 3);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 2);
    it++;
    BOOST_CHECK_EQUAL(*it, 3);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }

  // Clean up
  std::vector<DiffLine*>::iterator it;
  for(it = textLines.begin(); it != textLines.end(); it++)
  {
    delete *it;
  }

  textLines.clear();
}


/**
 * test_TextSelectionExtended
 *
 * Test the methods TextSelection::startSelection(..) and
 * TextSelection::updateSelection(..)
 *
 * See Excel sheet
 *   'text_selection_algorithm.xlsx' in other repository Python-Library,
 * directory algo, for a visual of this test.
 */
BOOST_AUTO_TEST_CASE( test_TextSelectionExtended )
{
  DiffLine* pLine1 = new DiffLine("The first line");
  DiffLine* pLine2 = new DiffLine("The second line");
  DiffLine* pLine3 = new DiffLine("third line");
  DiffLine* pLine4 = new DiffLine("line four");
  DiffLine* pLine5 = new DiffLine("Line no. 5");
  DiffLine* pLine6 = new DiffLine("he she it them our");
  DiffLine* pLine7 = new DiffLine("limit_line_id() tests need this");
  DiffLine* pLine8 = new DiffLine("and this");

  std::vector<DiffLine*> textLines;
  textLines.push_back(pLine1);
  textLines.push_back(pLine2);
  textLines.push_back(pLine3);
  textLines.push_back(pLine4);
  textLines.push_back(pLine5);
  textLines.push_back(pLine6);
  textLines.push_back(pLine7);
  textLines.push_back(pLine8);

  try
  {
    long column;
    unsigned long line;
    std::list<long>::const_iterator it;
    DynamicSelection dynamicSelection(textLines);
    
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(2, 4), 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNextSelectionStart(2, 0), -1);
    
    // 13) New test: Start a new dynamicSelection on lineId=4
    dynamicSelection.startSelection(4, 3);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 3);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    // 14) Update the dynamicSelection for *two* lines to top
    dynamicSelection.updateSelection(2, 3);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 3);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 7);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 3);

    // Updating another 2 lines to top and then two lines to bottom and it
    // should look the same

    dynamicSelection.updateSelection(0, 3);
    dynamicSelection.updateSelection(2, 3);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 3);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 7);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 3);

    // Updating another 2 lines to bottom is the start potion again
    dynamicSelection.updateSelection(4, 3);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 3);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    // 15) From the new start position (2, 6) ...
    dynamicSelection.startSelection(2, 6);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 6);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // ...select 2 lines to the bottom
    dynamicSelection.updateSelection(4, 6);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 6);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 6);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // 16) And another 2 lines  to the bottom
    dynamicSelection.updateSelection(6, 6);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 6);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 10);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 18);

    line = 6;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 6);

    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Now two lines upward and it should be the same state as in 15)
    dynamicSelection.updateSelection(4, 6);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 6);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 6);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // 17) From the new start position (2, 4) ...
    dynamicSelection.startSelection(2, 4);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // ... select 1 line to the top and 3 columns to the right
    dynamicSelection.updateSelection(1, 7);

    line = 0;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 8);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // 18) Then select 3 lines to the bottom
    dynamicSelection.updateSelection(4, 7);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 6);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 7);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // 19) Then select 5 lines to the top
    dynamicSelection.updateSelection(0, 7);

    line = 0;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 7);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 15);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Start a new dynamicSelection
    dynamicSelection.startSelection(4, 6);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 7);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 0);
    it++;
    BOOST_CHECK_EQUAL(*it, 1);
    it++;
    BOOST_CHECK_EQUAL(*it, 2);
    it++;
    BOOST_CHECK_EQUAL(*it, 3);
    it++;
    BOOST_CHECK_EQUAL(*it, 4);
    it++;
    BOOST_CHECK_EQUAL(*it, 5);
    it++;
    BOOST_CHECK_EQUAL(*it, 6);

    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(4, 7);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 4);

    dynamicSelection.updateSelection(4, 2);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 4);

    dynamicSelection.updateSelection(1, 2);
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 4);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 1);
    it++;
    BOOST_CHECK_EQUAL(*it, 2);
    it++;
    BOOST_CHECK_EQUAL(*it, 3);
    it++;
    BOOST_CHECK_EQUAL(*it, 4);

    // 20) Bug detected: Start a new selection
    dynamicSelection.startSelection(4, 3);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 4);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 1);
    it++;
    BOOST_CHECK_EQUAL(*it, 2);
    it++;
    BOOST_CHECK_EQUAL(*it, 3);
    it++;
    BOOST_CHECK_EQUAL(*it, 4);

    dynamicSelection.clearUpdatedLineIds();
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);
    
    // Update the selection 2 to the left, one to the top and five to the right
    dynamicSelection.updateSelection(4, 1);
    dynamicSelection.updateSelection(3, 1);
    dynamicSelection.updateSelection(3, 6);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 6);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 3);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 3);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 3);
    it++;
    BOOST_CHECK_EQUAL(*it, 4);

    dynamicSelection.clearUpdatedLineIds();
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);

    // Update the selection 1 to the bottom
    dynamicSelection.updateSelection(4, 6);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 3);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 3);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 3);
    it++;
    BOOST_CHECK_EQUAL(*it, 4);

    // 21) Trying to find another bug
    dynamicSelection.startSelection(4, 3);
    dynamicSelection.clearUpdatedLineIds();
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);

    // Update the selection to (1, 7)
    dynamicSelection.updateSelection(1, 7);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 8);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 10);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 3);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 4);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 1);
    it++;
    BOOST_CHECK_EQUAL(*it, 2);
    it++;
    BOOST_CHECK_EQUAL(*it, 3);
    it++;
    BOOST_CHECK_EQUAL(*it, 4);
    dynamicSelection.clearUpdatedLineIds();

    // Update the selection to (6, 7)
    dynamicSelection.updateSelection(6, 7);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 3);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 7);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 18);

    line = 6;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 7);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 6);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 1);
    it++;
    BOOST_CHECK_EQUAL(*it, 2);
    it++;
    BOOST_CHECK_EQUAL(*it, 3);
    it++;
    BOOST_CHECK_EQUAL(*it, 4);
    it++;
    BOOST_CHECK_EQUAL(*it, 5);
    it++;
    BOOST_CHECK_EQUAL(*it, 6);
    dynamicSelection.clearUpdatedLineIds();

    // Update the selection back to (1, 7)
    dynamicSelection.updateSelection(1, 7);

    line = 1;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 8);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 10);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 9);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 3);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 6);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 1);
    it++;
    BOOST_CHECK_EQUAL(*it, 2);
    it++;
    BOOST_CHECK_EQUAL(*it, 3);
    it++;
    BOOST_CHECK_EQUAL(*it, 4);
    it++;
    BOOST_CHECK_EQUAL(*it, 5);
    it++;
    BOOST_CHECK_EQUAL(*it, 6);
    dynamicSelection.clearUpdatedLineIds();
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }

  // Clean up
  std::vector<DiffLine*>::iterator it;
  for(it = textLines.begin(); it != textLines.end(); it++)
  {
    delete *it;
  }

  textLines.clear();

}


/**
 * test_TextSelectionInLine
 *
 * Testing the text selection in ADiffView unveiled problems when
 * dynamicSelection in the same line is reduced. This is tested here to be
 * fixed.
 */
BOOST_AUTO_TEST_CASE( test_TextSelectionInLine )
{
  DiffLine* pLine0 = new DiffLine("The first line");
  DiffLine* pLine1 = new DiffLine("The second line");
  DiffLine* pLine2 = new DiffLine("third line");
  DiffLine* pLine3 = new DiffLine("line four");
  DiffLine* pLine4 = new DiffLine("Line no. 5");
  DiffLine* pLine5 = new DiffLine("he she it them our");
  DiffLine* pLine6 = new DiffLine("limit_line_id() tests need this");
  DiffLine* pLine7 = new DiffLine("and this");
  DiffLine* pLine8 = new DiffLine("");
  DiffLine* pLine9 = new DiffLine("blank line above");

  std::vector<DiffLine*> textLines;
  textLines.push_back(pLine0);
  textLines.push_back(pLine1);
  textLines.push_back(pLine2);
  textLines.push_back(pLine3);
  textLines.push_back(pLine4);
  textLines.push_back(pLine5);
  textLines.push_back(pLine6);
  textLines.push_back(pLine7);
  textLines.push_back(pLine8);
  textLines.push_back(pLine9);

  try
  {
    long column;
    unsigned long line;
    std::list<long>::const_iterator it;
    DynamicSelection dynamicSelection(textLines);
    
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(2, 4), 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNextSelectionStart(2, 0), -1);
    
    // Start a new dynamicSelection
    dynamicSelection.startSelection(6, 7);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 6;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Update the dynamicSelection 6 chars to the right
    dynamicSelection.updateSelection(6, 13);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 6;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 6);

    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Update the dynamicSelection 4 chars to the left
    dynamicSelection.updateSelection(6, 9);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 6;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 2);

    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Update the dynamicSelection another 2 chars to the left; should be like
    // the start position again
    dynamicSelection.updateSelection(6, 7);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 6;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Update the dynamicSelection another 4 chars to the left;
    dynamicSelection.updateSelection(6, 3);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 6;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 3);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Finally update the dynamicSelection another 5 chars to the right;
    dynamicSelection.updateSelection(6, 8);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 6;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 7);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 1);

    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Clear the dynamicSelection
    dynamicSelection.clear();
    dynamicSelection.clearUpdatedLineIds();
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);

    // Remember the last three lines:
    //     DiffLine* pLine7 = new DiffLine("and this");
    //     DiffLine* pLine8 = new DiffLine("");
    //     DiffLine* pLine9 = new DiffLine("blank line above");

    // Start a dynamicSelection at the first char of line 7
    dynamicSelection.startSelection(7, 0);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 7);

    // Now select the non existing char 0 of line 8 (line 8 is empty)
    dynamicSelection.updateSelection(8, 0);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 7);

    it++;
    BOOST_CHECK_EQUAL(*it, 8);

    // Now check what's really selected
    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 8);

    line = 8;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0); // 0 is the length of ""

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    dynamicSelection.clearUpdatedLineIds();

    // Now select the non existing char 2 of line 8 (line 8 is empty)
    dynamicSelection.updateSelection(8, 2);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);

    // Now check what's really selected
    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 8);

    line = 8;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0); // 0 is the length of ""

    // Again, remember the last three lines:
    //     DiffLine* pLine7 = new DiffLine("and this");
    //     DiffLine* pLine8 = new DiffLine("");
    //     DiffLine* pLine9 = new DiffLine("blank line above");

    // Start a dynamicSelection at the first char 0 of line 9
    dynamicSelection.clear();
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.startSelection(9, 0);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 9);

    // Now select the non existing char 0 of line 8 (line 8 is empty)
    dynamicSelection.updateSelection(8, 0);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 8);

    it++;
    BOOST_CHECK_EQUAL(*it, 9);

    // Now check what's really selected
    line = 8;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0); // 0 is the length of ""

    line = 9;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 2);
    dynamicSelection.clearUpdatedLineIds();

    // Now select the non existing char 2 of line 8 (line 8 is empty)
    dynamicSelection.updateSelection(8, 2);
    dynamicSelection.updateSelection(8, 2);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);

    // Now check what's really selected
    line = 8;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0); // 0 is the length of ""

    line = 9;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    // Finally, again with these lines:
    //     DiffLine* pLine7 = new DiffLine("and this");
    //     DiffLine* pLine8 = new DiffLine("");
    //     DiffLine* pLine9 = new DiffLine("blank line above");

    // Start a dynamicSelection at the non existing char 2 of line 8
    dynamicSelection.clear();
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.startSelection(8, 2);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 8);

    // Now check what's really selected
    line = 8;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0); // 0 is the length of ""

    // Update the dynamicSelection 3 chars to the right (non existing char 3 of line 8)
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(8, 3);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0); // Nothing changed

    // Now check what's really selected
    line = 8;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0); // 0 is the length of ""

    // Start a dynamicSelection at the char 4 of line 7
    dynamicSelection.clear();
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.startSelection(7, 4);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 7);

    // Now check what's really selected
    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    // Update 2 chars to the right
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(7, 6);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 7);

    // Now check what's really selected
    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 2);

    // Update 20 chars to the right
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(7, 26);

    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);
    it = dynamicSelection.getUpdatedLineIds().begin();
    BOOST_CHECK_EQUAL(*it, 7);

    // Now check what's really selected
    line = 7;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4); // the limit to line length worked
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }

  // Clean up
  std::vector<DiffLine*>::iterator it;
  for(it = textLines.begin(); it != textLines.end(); it++)
  {
    delete *it;
  }

  textLines.clear();

}

/**
 * test_TextSelectionMultiCall
 *
 * Testing the text selection in ADiffView unveiled problems when e.g.
 * dynamicSelection started upward and changed downward below the starting
 * line. Now while the selecting mouse button still was pressed
 * updateSelection() was called multiple times with equal
 * parameters and unveiled strange effects.
 */
BOOST_AUTO_TEST_CASE( test_TextSelectionMultiCall )
{
  DiffLine* pLine1 = new DiffLine("The first line");
  DiffLine* pLine2 = new DiffLine("The second line");
  DiffLine* pLine3 = new DiffLine("third line");
  DiffLine* pLine4 = new DiffLine("line four");
  DiffLine* pLine5 = new DiffLine("Line no. 5");
  DiffLine* pLine6 = new DiffLine("he she it them our");
  DiffLine* pLine7 = new DiffLine("limit_line_id() tests need this");
  DiffLine* pLine8 = new DiffLine("and this");

  std::vector<DiffLine*> textLines;
  textLines.push_back(pLine1);
  textLines.push_back(pLine2);
  textLines.push_back(pLine3);
  textLines.push_back(pLine4);
  textLines.push_back(pLine5);
  textLines.push_back(pLine6);
  textLines.push_back(pLine7);
  textLines.push_back(pLine8);

  try
  {
    long column;
    unsigned long line;
    std::list<int>::const_iterator it;
    DynamicSelection dynamicSelection(textLines);
    
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(2, 4), 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNextSelectionStart(2, 0), -1);
    
    // Start a new dynamicSelection at line 4, column 5
    dynamicSelection.startSelection(4, 5);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Update the dynamicSelection one top...
    dynamicSelection.updateSelection(3, 5);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 5);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // ...and call the update multiple times
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(3, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);

    dynamicSelection.updateSelection(3, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 5);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Start a new dynamicSelection at line 4, column 5
    dynamicSelection.startSelection(4, 5);
    dynamicSelection.clearUpdatedLineIds();

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 0);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // 'Update' the dynamicSelection multiple times to the start position line 4, column 5
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(4, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);
    dynamicSelection.updateSelection(4, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);

    // Update the dynamicSelection one top...
    dynamicSelection.updateSelection(3, 5);

    line = 2;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    line = 4;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 0);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 5);

    line = 5;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, -1);

    // Update the dynamicSelection by one to right ...
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(3, 6);
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);

    // The same position again should cause no update
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(3, 6);
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 6);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 3);

    // And back one to the left...
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(3, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 1);

    // The same position again should cause no update
    dynamicSelection.clearUpdatedLineIds();
    dynamicSelection.updateSelection(3, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getUpdatedLineIds().size(), 0);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 4);

    
    // And again one to the left...
    dynamicSelection.updateSelection(3, 4);
    dynamicSelection.updateSelection(3, 4);

    line = 3;
    column = dynamicSelection.getNextSelectionStart(line, 0);
    BOOST_CHECK_EQUAL(column, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.getNumMarkedChars(line, column), 5);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }

  // Clean up
  std::vector<DiffLine*>::iterator it;
  for(it = textLines.begin(); it != textLines.end(); it++)
  {
    delete *it;
  }

  textLines.clear();
}


/**
 * test_TextSelectionIsSelected
 *
 * Testing the TextSelection::isSelected(...) method
 */
BOOST_AUTO_TEST_CASE( test_TextSelectionIsSelected )
{
  DiffLine* pLine1 = new DiffLine("The first line");
  DiffLine* pLine2 = new DiffLine("The second line");
  DiffLine* pLine3 = new DiffLine("third line");
  DiffLine* pLine4 = new DiffLine("line four");
  DiffLine* pLine5 = new DiffLine("Line no. 5");
  DiffLine* pLine6 = new DiffLine("he she it them our");
  DiffLine* pLine7 = new DiffLine("limit_line_id() tests need this");
  DiffLine* pLine8 = new DiffLine("and this");

  std::vector<DiffLine*> textLines;
  textLines.push_back(pLine1);
  textLines.push_back(pLine2);
  textLines.push_back(pLine3);
  textLines.push_back(pLine4);
  textLines.push_back(pLine5);
  textLines.push_back(pLine6);
  textLines.push_back(pLine7);
  textLines.push_back(pLine8);

  try
  {
    long column;
    unsigned long line;
    std::list<int>::const_iterator it;
    DynamicSelection dynamicSelection(textLines);
    
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 4), false);
    dynamicSelection.startSelection(2, 4);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 4), true);
    
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 8), false);
    dynamicSelection.updateSelection(2, 9);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 8), true);

    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 1), false);
    dynamicSelection.updateSelection(1, 5);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 8), false);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 7), false);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 6), false);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 5), false);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 4), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 3), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 2), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 1), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 0), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 15), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 16), false);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 14), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 13), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 12), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 11), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 10), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 9), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 8), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 7), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 6), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 5), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 4), false);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 3), false);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 2), false);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 1), false);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(1, 0), false);

    dynamicSelection.updateSelection(5, 1);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(2, 1), false);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(3, 1), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(4, 1), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(5, 1), true);
    BOOST_CHECK_EQUAL(dynamicSelection.isSelected(6, 1), false);

    StaticSelection selection(textLines);
    BOOST_CHECK_EQUAL(selection.isSelected(2, 4), false);
    selection.addBlock(2, 2, 4);
    BOOST_CHECK_EQUAL(selection.isSelected(2, 4), true);

    BOOST_CHECK_EQUAL(selection.isSelected(1, 4), false);
    selection.addBlock(1, 2, 4);
    BOOST_CHECK_EQUAL(selection.isSelected(1, 4), true);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }

  // Clean up
  std::vector<DiffLine*>::iterator it;
  for(it = textLines.begin(); it != textLines.end(); it++)
  {
    delete *it;
  }

  textLines.clear();

}




/**
 * testcase_32
 *
 * Test the methods SelectableDiffFile::getNumNormalChars() and
 * getNumMarkedChars() of SelectableDiffFile according the algorithm 
 * developed in Excel sheet 'ADiffView - selected text and scrolling.xlsx'
 */
BOOST_AUTO_TEST_CASE( test_32_SelectableDiffFile )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_32_numChars_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_32_numChars_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 2);
    BOOST_CHECK_EQUAL(diffB.getNumLines(), 2);

    SelectableDiffFile diffASelectable(diffA);
    diffASelectable.addSearchResultSelectionBlock(0, 7, 14);
    diffASelectable.addSearchResultSelectionBlock(0, 26, 28);

    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 0), 7);
    BOOST_CHECK_EQUAL(diffASelectable.getNumMarkedChars(0, 7), 8);
    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 15), 11);

    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 5), 2);
    BOOST_CHECK_EQUAL(diffASelectable.getNumMarkedChars(0, 7), 8);
    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 15), 11);

    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 7), 0);
    BOOST_CHECK_EQUAL(diffASelectable.getNumMarkedChars(0, 7), 8);
    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 15), 11);

    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 10), 0);
    BOOST_CHECK_EQUAL(diffASelectable.getNumMarkedChars(0, 10), 5);
    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 15), 11);
    BOOST_CHECK_EQUAL(diffASelectable.getNumMarkedChars(0, 26), 3);

    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 16), 10);
    BOOST_CHECK_EQUAL(diffASelectable.getNumMarkedChars(0, 26), 3);
    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 29), 22);

    BOOST_CHECK_EQUAL(diffASelectable.getNumNormalChars(0, 30), 21);

  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


BOOST_AUTO_TEST_CASE( testcase_search_algorithm_basic )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_23_RealLifeApp-left.cs",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_23_RealLifeApp-right.cs",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();


    size_t numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 11);

    const char* pSearchFor = "Software";
    DiffFileSearchEngine searchEngine(diffA, 
                                      diffB, 
                                      "Software",
                                      false,
                                      SearchLocation::SL_BothFiles);

    BOOST_CHECK_EQUAL(searchEngine.getNumResults(), 6);

    DiffFileSearchResult* pSearchResult = searchEngine.getNextResult(0);
    BOOST_CHECK(pSearchResult != NULL);
    
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 53);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 113);


  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


BOOST_AUTO_TEST_CASE( search_algorithm_extended_1 )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_33_search_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_33_search_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();


    size_t numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 4);

    DiffFileSearchEngine searchEngine(diffA, 
                                      diffB, 
                                      "test",
                                      false,
                                      SearchLocation::SL_BothFiles);

    BOOST_CHECK_EQUAL(searchEngine.getNumResults(), 4);

    DiffFileSearchResult* pSearchResult = searchEngine.getNextResult(0);
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 2);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 18);

    pSearchResult = searchEngine.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::RightFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 2);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 18);

    pSearchResult = searchEngine.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::RightFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 2);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 23);

    pSearchResult = searchEngine.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::RightFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 2);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 28);

    DiffFileSearchEngine searchEngine2(diffA, 
                                       diffB, 
                                       "can",
                                       false,
                                       SearchLocation::SL_BothFiles);

    BOOST_CHECK_EQUAL(searchEngine2.getNumResults(), 4);

    pSearchResult = searchEngine2.getNextResult(0);
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 0);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 8);

    pSearchResult = searchEngine2.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);

    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 0);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 12);

    // Now searching for the word 'left'
    DiffFileSearchEngine searchEngine3(diffA, 
                                       diffB, 
                                       "left",
                                       false,
                                       SearchLocation::SL_BothFiles);

    BOOST_CHECK_EQUAL(searchEngine3.getNumResults(), 4);

    // The first result when starting from line 0 should be in the left
    // file on lineId = 4
    pSearchResult = searchEngine3.getNextResult(0);
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 4);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 29);

    // The next result should be in the right file 
    pSearchResult = searchEngine3.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::RightFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 4);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 29);

    // The next result should be in the left file again
    pSearchResult = searchEngine3.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 6);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 0);

    // But now consider we have (virtually) scrolled some lines down.
    // Now, the first search result we want should be past line 4. So,
    // the lineId=6 at first charPosition should be the result.
    pSearchResult = searchEngine3.getNextResult(5);
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 6);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 0);

    // And the next result should be four lines below
    pSearchResult = searchEngine3.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 10);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 0);

    // And then there should be no next result
    pSearchResult = searchEngine3.getNextResult();
    BOOST_CHECK(pSearchResult == NULL);

  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


BOOST_AUTO_TEST_CASE( search_algorithm_case_ignored )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_33_search_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_33_search_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    size_t numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 4);

    DiffFileSearchEngine searchEngine(diffA, 
                                      diffB, 
                                      "Left",
                                      true,
                                      SearchLocation::SL_BothFiles);

    BOOST_CHECK_EQUAL(searchEngine.getNumResults(), 5);

    DiffFileSearchResult* pSearchResult = searchEngine.getNextResult(0);
    BOOST_CHECK(pSearchResult != NULL);
    BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
    BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 4);
    BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 29);

    pSearchResult = searchEngine.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::RightFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 4);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 29);
    }


    pSearchResult = searchEngine.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 6);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 0);
    }


    pSearchResult = searchEngine.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 8);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 0);
    }


    pSearchResult = searchEngine.getNextResult();
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 10);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 0);
    }
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


BOOST_AUTO_TEST_CASE( search_algorithm_get_prev_result )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_33_search_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_33_search_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    size_t numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 4);

    DiffFileSearchEngine searchEngine(diffA, 
                                      diffB, 
                                      "Left",
                                      true,
                                      SearchLocation::SL_BothFiles);

    BOOST_CHECK_EQUAL(searchEngine.getNumResults(), 5);

    DiffFileSearchResult* pSearchResult = searchEngine.getPrevResult(8);
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 6);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 0);
    }
    
    pSearchResult = searchEngine.getPrevResult();
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::RightFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 4);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 29);
    }

    pSearchResult = searchEngine.getPrevResult();
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 4);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 29);
    }
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


DiffFileSearchResult* getNextResultFromPage(DiffFileSearchEngine* m_pSearchEngine,
                                            size_t lineId)
{
  DiffFileSearchResult* m_pFormerResult = m_pSearchEngine->getCurrentResult();
  DiffFileSearchResult* pResult = m_pSearchEngine->getNextResult(lineId);
  
  if((pResult != NULL) && (m_pFormerResult != NULL) &&
     (pResult->getLineId() == m_pFormerResult->getLineId()))
  {
    // New result is on the same line as former result. If Necessary,
    // repeat getNextResult until new result is after former result on
    // this line.
    while(!m_pFormerResult->isBefore(pResult))
    {
      pResult = m_pSearchEngine->getNextResult();
      if(pResult == NULL)
      {
        break;
      }
    }
  }

  return pResult;
}


BOOST_AUTO_TEST_CASE( search_algorithm_get_next_and_prev_from_current_position )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_33_search_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_33_search_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    size_t numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 4);

    DiffFileSearchEngine searchEngine(diffA, 
                                      diffB, 
                                      "left",
                                      true,
                                      SearchLocation::SL_BothFiles);

    BOOST_CHECK_EQUAL(searchEngine.getNumResults(), 5);

    DiffFileSearchResult* pSearchResult = getNextResultFromPage(&searchEngine,
                                                                0);
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 4);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 29);
    }
    
    pSearchResult = getNextResultFromPage(&searchEngine,
                                          pSearchResult->getLineId());
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::RightFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 4);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 29);
    }

    pSearchResult = getNextResultFromPage(&searchEngine,
                                          pSearchResult->getLineId());
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 6);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 0);
    }

    pSearchResult = getNextResultFromPage(&searchEngine,
                                          pSearchResult->getLineId());
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 8);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 0);
    }

    //
    // Now the users scrolls three lines up to lineId=5, so the next
    // result should be that one in line six
    //
    pSearchResult = getNextResultFromPage(&searchEngine,
                                          5);
    BOOST_CHECK(pSearchResult != NULL);
    if(pSearchResult != NULL)
    {
      BOOST_CHECK_EQUAL(pSearchResult->getLocation(), DiffFileSearchResult::LeftFile);
      BOOST_CHECK_EQUAL(pSearchResult->getLineId(), 6);
      BOOST_CHECK_EQUAL(pSearchResult->getCharId(), 0);
    }

  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}

BOOST_AUTO_TEST_CASE( test_DiffLine_getTextPositionInfo_1 )
{
  DiffLine line("\tThis\tis\ta\ttab\ttest\tyeah", 
                DiffLine::Normal,
                "001");

  BOOST_CHECK_EQUAL(line.getNumChars(), 24);

  TextPositionInfo nominal[] =
  {
    // {numRemainingSpaces, numRemainingChars, srcTextColumn}
    { 8, 0, 0 },
    { 7, 0, 0 },
    { 6, 0, 0 },
    { 5, 0, 0 },
    { 4, 0, 0 },
    { 3, 0, 0 },
    { 2, 0, 0 },
    { 1, 0, 0 },
    { 0, 4, 1 },
    { 0, 3, 2 },
    { 0, 2, 3 },
    { 0, 1, 4 },
    { 4, 0, 5 },
    { 3, 0, 5 },
    { 2, 0, 5 },
    { 1, 0, 5 },
    { 0, 2, 6 },
    { 0, 1, 7 },
    { 6, 0, 8 },
    { 5, 0, 8 },
    { 4, 0, 8 },
    { 3, 0, 8 },
    { 2, 0, 8 },
    { 1, 0, 8 },
    { 0, 1, 9 },
    { 7, 0, 10 }, 
    { 6, 0, 10 }, 
    { 5, 0, 10 }, 
    { 4, 0, 10 }, 
    { 3, 0, 10 }, 
    { 2, 0, 10 }, 
    { 1, 0, 10 }, 
    { 0, 3, 11 }, 
    { 0, 2, 12 }, 
    { 0, 1, 13 }, 
    { 5, 0, 14 }, 
    { 4, 0, 14 }, 
    { 3, 0, 14 }, 
    { 2, 0, 14 }, 
    { 1, 0, 14 }, 
    { 0, 4, 15 }, 
    { 0, 3, 16 }, 
    { 0, 2, 17 }, 
    { 0, 1, 18 }, 
    { 4, 0, 19 }, 
    { 3, 0, 19 }, 
    { 2, 0, 19 }, 
    { 1, 0, 19 }, 
    { 0, 4, 20 }, 
    { 0, 3, 21 }, 
    { 0, 2, 22 }, 
    { 0, 1, 23 }, 
    { 0, 0, 24 },
  };

  size_t arraySize = sizeof(nominal) / sizeof(nominal[0]);
  for(size_t i = 0; i < arraySize; i++)
  {
    TextPositionInfo actual;
    line.getTextPositionInfo(&actual, i, 8);
    BOOST_CHECK_EQUAL(actual.numRemainingSpaces, nominal[i].numRemainingSpaces);
    BOOST_CHECK_EQUAL(actual.numRemainingChars, nominal[i].numRemainingChars);
    BOOST_CHECK_EQUAL(actual.srcTextColumn, nominal[i].srcTextColumn);
  }

}

BOOST_AUTO_TEST_CASE( test_DiffLine_getTextPositionInfo_2 )
{
  DiffLine line("\tXDEF\t_SysBase", 
                DiffLine::Normal,
                "001");

  BOOST_CHECK_EQUAL(line.getNumChars(), 14);

  TextPositionInfo nominal[] =
  {
    // {numRemainingSpaces, numRemainingChars, srcTextColumn}
    { 8, 0, 0  },
    { 7, 0, 0  },
    { 6, 0, 0  },
    { 5, 0, 0  },
    { 4, 0, 0  },
    { 3, 0, 0  },
    { 2, 0, 0  },
    { 1, 0, 0  },
    { 0, 4, 1  },
    { 0, 3, 2  },
    { 0, 2, 3  },
    { 0, 1, 4  },
    { 4, 0, 5  },
    { 3, 0, 5  },
    { 2, 0, 5  },
    { 1, 0, 5  },
    { 0, 8, 6  },
    { 0, 7, 7  },
    { 0, 6, 8  },
    { 0, 5, 9  },
    { 0, 4, 10 },
    { 0, 3, 11 },
    { 0, 2, 12 },
    { 0, 1, 13 },
    { 0, 0, 14 },
  };

  size_t arraySize = sizeof(nominal) / sizeof(nominal[0]);
  for(size_t i = 0; i < arraySize; i++)
  {
    TextPositionInfo actual;
    line.getTextPositionInfo(&actual, i, 8);
    BOOST_CHECK_EQUAL(actual.numRemainingSpaces, nominal[i].numRemainingSpaces);
    BOOST_CHECK_EQUAL(actual.numRemainingChars, nominal[i].numRemainingChars);
    BOOST_CHECK_EQUAL(actual.srcTextColumn, nominal[i].srcTextColumn);
  }

}


BOOST_AUTO_TEST_CASE( test_DiffLine_getTextPositionInfo_3 )
{
  DiffLine line("\t\tdc.w\tdmacon,$0020\t;Disable sprites", 
                DiffLine::Normal,
                "001");

  BOOST_CHECK_EQUAL(line.getNumChars(), 36);

  TextPositionInfo nominal[] =
  {
    // {numRemainingSpaces, numRemainingChars, srcTextColumn}
    { 8,  0,  0 },
    { 7,  0,  0 },
    { 6,  0,  0 },
    { 5,  0,  0 },
    { 4,  0,  0 },
    { 3,  0,  0 },
    { 2,  0,  0 },
    { 1,  0,  0 },
    { 8,  0,  1 },
    { 7,  0,  1 },
    { 6,  0,  1 },
    { 5,  0,  1 },
    { 4,  0,  1 },
    { 3,  0,  1 },
    { 2,  0,  1 },
    { 1,  0,  1 },
    { 0,  4,  2 },
    { 0,  3,  3 },
    { 0,  2,  4 },
    { 0,  1,  5 },
    { 4,  0,  6 },
    { 3,  0,  6 },
    { 2,  0,  6 },
    { 1,  0,  6 },
    { 0, 12,  7 },
    { 0, 11,  8 },
    { 0, 10,  9 },
    { 0,  9, 10 },
    { 0,  8, 11 },
    { 0,  7, 12 },
    { 0,  6, 13 },
    { 0,  5, 14 },
    { 0,  4, 15 },
    { 0,  3, 16 },
    { 0,  2, 17 },
    { 0,  1, 18 },
    { 4,  0, 19 },
    { 3,  0, 19 },
    { 2,  0, 19 },
    { 1,  0, 19 },
    { 0, 16, 20 },
    { 0, 15, 21 },
    { 0, 14, 22 },
    { 0, 13, 23 },
    { 0, 12, 24 },
    { 0, 11, 25 },
    { 0, 10, 26 },
    { 0,  9, 27 },
    { 0,  8, 28 },
    { 0,  7, 29 },
    { 0,  6, 30 },
    { 0,  5, 31 },
    { 0,  4, 32 },
    { 0,  3, 33 },
    { 0,  2, 34 },
    { 0,  1, 35 },
    { 0,  0, 36 },
  };

  size_t arraySize = sizeof(nominal) / sizeof(nominal[0]);
  for(size_t i = 0; i < arraySize; i++)
  {
    TextPositionInfo actual;
    line.getTextPositionInfo(&actual, i, 8);
    BOOST_CHECK_EQUAL(actual.numRemainingSpaces, nominal[i].numRemainingSpaces);
    BOOST_CHECK_EQUAL(actual.numRemainingChars, nominal[i].numRemainingChars);
    BOOST_CHECK_EQUAL(actual.srcTextColumn, nominal[i].srcTextColumn);
  }
}


BOOST_AUTO_TEST_CASE( test_DiffLine_getRenderColumn )
{
  const int TAB_WIDTH = 8;
  DiffLine line("\tThis\tis\ta\ttab\ttest\tyeah", 
                DiffLine::Normal,
                "001");

  BOOST_CHECK_EQUAL(line.getNumChars(), 24);
  BOOST_CHECK_EQUAL(line.getRenderColumn(0, TAB_WIDTH), 0);
  BOOST_CHECK_EQUAL(line.getRenderColumn(1, TAB_WIDTH), 8);
  BOOST_CHECK_EQUAL(line.getRenderColumn(2, TAB_WIDTH), 9);
  BOOST_CHECK_EQUAL(line.getRenderColumn(3, TAB_WIDTH), 10);
  BOOST_CHECK_EQUAL(line.getRenderColumn(4, TAB_WIDTH), 11);
  BOOST_CHECK_EQUAL(line.getRenderColumn(5, TAB_WIDTH), 12);
  BOOST_CHECK_EQUAL(line.getRenderColumn(6, TAB_WIDTH), 16); // position of 'is' on TAB_WIDTH = 8
  BOOST_CHECK_EQUAL(line.getRenderColumn(7, TAB_WIDTH), 17);
  BOOST_CHECK_EQUAL(line.getRenderColumn(8, TAB_WIDTH), 18);
  BOOST_CHECK_EQUAL(line.getRenderColumn(9, TAB_WIDTH), 24);
  BOOST_CHECK_EQUAL(line.getRenderColumn(10, TAB_WIDTH), 25);
  BOOST_CHECK_EQUAL(line.getRenderColumn(11, TAB_WIDTH), 32); // position of 'tab' on TAB_WIDTH = 8
  BOOST_CHECK_EQUAL(line.getRenderColumn(12, TAB_WIDTH), 33);
  BOOST_CHECK_EQUAL(line.getRenderColumn(13, TAB_WIDTH), 34);
  BOOST_CHECK_EQUAL(line.getRenderColumn(14, TAB_WIDTH), 35);
  BOOST_CHECK_EQUAL(line.getRenderColumn(15, TAB_WIDTH), 40);
  BOOST_CHECK_EQUAL(line.getRenderColumn(16, TAB_WIDTH), 41);
  BOOST_CHECK_EQUAL(line.getRenderColumn(17, TAB_WIDTH), 42);
  BOOST_CHECK_EQUAL(line.getRenderColumn(18, TAB_WIDTH), 43);
  BOOST_CHECK_EQUAL(line.getRenderColumn(19, TAB_WIDTH), 44);
  BOOST_CHECK_EQUAL(line.getRenderColumn(20, TAB_WIDTH), 48);
  BOOST_CHECK_EQUAL(line.getRenderColumn(21, TAB_WIDTH), 49);
  BOOST_CHECK_EQUAL(line.getRenderColumn(22, TAB_WIDTH), 50);
  BOOST_CHECK_EQUAL(line.getRenderColumn(23, TAB_WIDTH), 51);
}


BOOST_AUTO_TEST_CASE( test_DiffLine_getDocumentColumn_1 )
{
  const int TAB_WIDTH = 8;
  DiffLine line("This is a test without TABs", 
                DiffLine::Normal,
                "001");

  BOOST_CHECK_EQUAL(line.getNumChars(), 27);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(0, TAB_WIDTH), 0);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(1, TAB_WIDTH), 1);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(2, TAB_WIDTH), 2);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(3, TAB_WIDTH), 3);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(4, TAB_WIDTH), 4);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(5, TAB_WIDTH), 5);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(6, TAB_WIDTH), 6);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(7, TAB_WIDTH), 7);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(8, TAB_WIDTH), 8);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(9, TAB_WIDTH), 9);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(10, TAB_WIDTH), 10);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(11, TAB_WIDTH), 11);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(12, TAB_WIDTH), 12);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(13, TAB_WIDTH), 13);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(14, TAB_WIDTH), 14);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(15, TAB_WIDTH), 15);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(16, TAB_WIDTH), 16);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(17, TAB_WIDTH), 17);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(18, TAB_WIDTH), 18);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(19, TAB_WIDTH), 19);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(20, TAB_WIDTH), 20);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(21, TAB_WIDTH), 21);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(22, TAB_WIDTH), 22);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(23, TAB_WIDTH), 23);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(24, TAB_WIDTH), 24);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(25, TAB_WIDTH), 25);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(26, TAB_WIDTH), 26);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(27, TAB_WIDTH), 27);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(28, TAB_WIDTH), 27);
}


BOOST_AUTO_TEST_CASE( test_DiffLine_getDocumentColumn_2 )
{
  const int TAB_WIDTH = 8;
  DiffLine line("\tThis\tis\ta\ttab\ttest\tyeah", 
                DiffLine::Normal,
                "001");

  BOOST_CHECK_EQUAL(line.getNumChars(), 24);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(0, TAB_WIDTH), 0);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(1, TAB_WIDTH), 0);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(2, TAB_WIDTH), 0);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(3, TAB_WIDTH), 0);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(4, TAB_WIDTH), 0);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(5, TAB_WIDTH), 0);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(6, TAB_WIDTH), 0);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(7, TAB_WIDTH), 0);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(8, TAB_WIDTH), 1);     // The 'T' of 'This'
  BOOST_CHECK_EQUAL(line.getDocumentColumn(9, TAB_WIDTH), 2);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(10, TAB_WIDTH), 3);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(11, TAB_WIDTH), 4);   // The 's' of 'This'  BOOST_CHECK_EQUAL(line.getDocumentColumn(12, TAB_WIDTH), 5);   // 4 more to fulfill the TAB width of 8 after 'This'
  BOOST_CHECK_EQUAL(line.getDocumentColumn(13, TAB_WIDTH), 5);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(14, TAB_WIDTH), 5);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(15, TAB_WIDTH), 5);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(16, TAB_WIDTH), 6);   // The 'i'
  BOOST_CHECK_EQUAL(line.getDocumentColumn(17, TAB_WIDTH), 7);   // The 's'
  BOOST_CHECK_EQUAL(line.getDocumentColumn(18, TAB_WIDTH), 8);   // 46 more to fulfill the TAB after 'is'
  BOOST_CHECK_EQUAL(line.getDocumentColumn(19, TAB_WIDTH), 8);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(20, TAB_WIDTH), 8);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(21, TAB_WIDTH), 8);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(22, TAB_WIDTH), 8);
  BOOST_CHECK_EQUAL(line.getDocumentColumn(23, TAB_WIDTH), 8);
}


unsigned long getMaxRenderedLineLength(DiffFileBase& m_LeftDiffFile, 
                                       DiffFileBase& m_RightDiffFile,
                                       unsigned long tabSize)
{
  // Determine the max. line length in chars. Can either be found in the
  // left or in the right file.
  unsigned long m_MaxLineLength = 0;
  unsigned long numChars;
  unsigned long renderColumn;
  for(size_t i = 0; i < m_LeftDiffFile.getNumLines(); i++)
  {
    numChars = m_LeftDiffFile[i]->getNumChars();
    renderColumn = m_LeftDiffFile[i]->getRenderColumn(numChars, tabSize);
    if(renderColumn > m_MaxLineLength)
    {
      m_MaxLineLength = renderColumn;
    }
  }

  for(size_t i = 0; i < m_RightDiffFile.getNumLines(); i++)
  {
    numChars = m_RightDiffFile[i]->getNumChars();
    renderColumn = m_RightDiffFile[i]->getRenderColumn(numChars, tabSize);
    if(renderColumn > m_MaxLineLength)
    {
      m_MaxLineLength = renderColumn;
    }
  }

  return m_MaxLineLength;
}

BOOST_AUTO_TEST_CASE( testcase_DiffWindowTextArea_NonTabulatorText )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_23_RealLifeApp-left.cs",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_23_RealLifeApp-right.cs",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    size_t numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 11);

    size_t tabSize = 8;
    size_t maxNumChars = getMaxRenderedLineLength(diffA, diffB, tabSize);

    DiffWindowTextAreaMock textArea(diffA,  // The file
                                    true,   // line numbers enabled
                                    maxNumChars,
                                    tabSize);

    textArea.setSize(280, 250);
    long lineTop = 0; // Not needed

    size_t num = 0;
    char renderBuf[1024];

    num = textArea.renderLine(0, false, lineTop, 0, renderBuf);  
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}using RealLifeApp.DB;");

    num = textArea.renderLine(12, false, lineTop, 0, renderBuf); 
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}using static RealLifeApp.DB.Q");

    // Now scroll the text area left by 2 chars
    textArea.scrollLeft(2);


    // Now for each line render the two chars at the right
    num = textArea.renderLine(0, false, lineTop, -2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "");

    num = textArea.renderLine(12, false, lineTop, -2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}ue");

    // After all lines to be observed are re-rendered the x-value must
    // be increased. In the original non-mocked file this is done in 
    // renderLine() - but here it must be done manually.
    textArea.m_X += 2;

    // Scroll the text area back to 0
    textArea.scrollRight(2);

    // And check the again
    num = textArea.renderLine(0, false, lineTop, 2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}us");

    num = textArea.renderLine(12, false, lineTop, 2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}us");

    // After all lines to be observed are re-rendered the x-value must
    // be increased. In the original non-mocked file this is done in 
    // renderLine() - but here it must be done manually.
    textArea.m_X -= 2;
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


BOOST_AUTO_TEST_CASE( testcase_DiffWindowTextArea_TabulatorText )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_35_tabs_asm_left.asm",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_35_tabs_asm_right.asm",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    size_t numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 7);

    size_t tabSize = 8;
    size_t maxNumChars = getMaxRenderedLineLength(diffA, diffB, tabSize);

    DiffWindowTextAreaMock textArea(diffA,  // The file
                                    true,   // line numbers enabled
                                    maxNumChars,
                                    tabSize);

    textArea.setSize(280, 250);
    long lineTop = 0; // Not needed

    size_t num = 0;
    char renderBuf[1024];

    num = textArea.renderLine(0, true, lineTop, 0, renderBuf);
    BOOST_CHECK_EQUAL(num, 4);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}        {NRMCOL}XDEF{NRMCOL}    {NRMCOL}_SysBase");

    num = textArea.renderLine(185, true, lineTop, 0, renderBuf);
    BOOST_CHECK_EQUAL(num, 4); 
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}        {NRMCOL}move.w{NRMCOL}  {NRMCOL}#%000010011111");

    // Now scroll the text area left by 2 chars
    textArea.scrollLeft(2);


    // Now for each line render the two chars at the right
    num = textArea.renderLine(0, false, lineTop, -2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "");

    num = textArea.renderLine(185, false, lineTop, -2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}00");

    // After all lines to be observed are re-rendered the x-value must
    // be increased. In the original non-mocked file this is done in 
    // renderLine() - but here it must be done manually.
    textArea.m_X += 2;

    // Scroll the text area back to 0
    textArea.scrollRight(2);

    // And check again
    num = textArea.renderLine(0, false, lineTop, 2, renderBuf);    // [DEBUG MEAS] OK:   1 x getTextPositionInfo()
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}  ");

    num = textArea.renderLine(185, false, lineTop, 2, renderBuf);   // [DEBUG MEAS] OK:   1 x getTextPositionInfo()
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}  ");

    // After all lines to be observed are re-rendered the x-value must
    // be increased. In the original non-mocked file this is done in 
    // renderLine() - but here it must be done manually.
    textArea.m_X -= 2;
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


BOOST_AUTO_TEST_CASE( testcase_DiffWindowTextArea_NonTabulator_MARKED_Text )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_23_RealLifeApp-left.cs",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_23_RealLifeApp-right.cs",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    size_t numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 11);

    size_t tabSize = 8;
    size_t maxNumChars = getMaxRenderedLineLength(diffA, diffB, tabSize);

    DiffWindowTextAreaMock textArea(diffA,  // The file
                                    true,   // line numbers enabled
                                    maxNumChars,
                                    tabSize);

    textArea.setSize(280, 250);
    long lineTop = 0; // Not needed

    size_t num = 0;
    char renderBuf[1024];

    textArea.addSearchResultSelectionBlock(0, 6, 16);
    textArea.addSearchResultSelectionBlock(12, 28, 33);

    num = textArea.renderLine(0, false, lineTop, 0, renderBuf);  
    BOOST_CHECK_EQUAL(num, 3);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}using {SELCOL}RealLifeApp{NRMCOL}.DB;");

    num = textArea.renderLine(12, false, lineTop, 0, renderBuf); 
    BOOST_CHECK_EQUAL(num, 2);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}using static RealLifeApp.DB.{SELCOL}Q");

    // // Now scroll the text area left by 2 chars
    textArea.scrollLeft(2);


    // Now for each line render the two chars at the right
    num = textArea.renderLine(0, false, lineTop, -2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "");

    num = textArea.renderLine(12, false, lineTop, -2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{SELCOL}ue");

    // After all lines to be observed are re-rendered the x-value must
    // be increased. In the original non-mocked file this is done in 
    // renderLine() - but here it must be done manually.
    textArea.m_X += 2;

    // Scroll the text area back to 0
    textArea.scrollRight(2);

    // And check the again
    num = textArea.renderLine(0, false, lineTop, 2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}us");

    num = textArea.renderLine(12, false, lineTop, 2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}us");

    // After all lines to be observed are re-rendered the x-value must
    // be increased. In the original non-mocked file this is done in 
    // renderLine() - but here it must be done manually.
    textArea.m_X -= 2;
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}



BOOST_AUTO_TEST_CASE( testcase_DiffWindowTextArea_Tabulator_MARKED_Text )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_35_tabs_asm_left.asm",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_35_tabs_asm_right.asm",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    size_t numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 7);

    size_t tabSize = 8;
    size_t maxNumChars = getMaxRenderedLineLength(diffA, diffB, tabSize);

    DiffWindowTextAreaMock textArea(diffA,  // The file
                                    true,   // line numbers enabled
                                    maxNumChars,
                                    tabSize);

    textArea.setSize(280, 250);
    long lineTop = 0; // Not needed

    size_t num = 0;
    char renderBuf[1024];

    textArea.addSearchResultSelectionBlock(0, 1, 4);
    textArea.addSearchResultSelectionBlock(185, 20, 23);

    num = textArea.renderLine(0, true, lineTop, 0, renderBuf);
    BOOST_CHECK_EQUAL(num, 4);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}        {SELCOL}XDEF{NRMCOL}    {NRMCOL}_SysBase");

    num = textArea.renderLine(185, true, lineTop, 0, renderBuf);
    BOOST_CHECK_EQUAL(num, 5); 
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}        {NRMCOL}move.w{NRMCOL}  {NRMCOL}#%0000100111{SELCOL}11");

    // Now scroll the text area left by 2 chars
    textArea.scrollLeft(2);


    // Now for each line render the two chars at the right
    num = textArea.renderLine(0, false, lineTop, -2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "");

    num = textArea.renderLine(185, false, lineTop, -2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{SELCOL}00");

    // After all lines to be observed are re-rendered the x-value must
    // be increased. In the original non-mocked file this is done in 
    // renderLine() - but here it must be done manually.
    textArea.m_X += 2;

    // Scroll the text area back to 0
    textArea.scrollRight(2);

    // And check again
    num = textArea.renderLine(0, false, lineTop, 2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}  ");

    num = textArea.renderLine(185, false, lineTop, 2, renderBuf);
    BOOST_CHECK_EQUAL(num, 1);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}  ");

    // After all lines to be observed are re-rendered the x-value must
    // be increased. In the original non-mocked file this is done in 
    // renderLine() - but here it must be done manually.
    textArea.m_X -= 2;
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}

BOOST_AUTO_TEST_CASE( testcase_DiffWindowTextArea_TabulatorRenderingProblem )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_35_tabs_asm_left.asm",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_35_tabs_asm_right.asm",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    size_t numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 7);

    size_t tabSize = 16;
    size_t maxNumChars = getMaxRenderedLineLength(diffA, diffB, tabSize);

    DiffWindowTextAreaMock textArea(diffA,  // The file
                                    true,   // line numbers enabled
                                    maxNumChars,
                                    tabSize);

    textArea.setSize(712, 250);
    long lineTop = 0; // Not needed

    size_t num = 0;
    char renderBuf[1024];

    num = textArea.renderLine(82, true, lineTop, 0, renderBuf);
    BOOST_CHECK_EQUAL(num, 4);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}                {NRMCOL}move.l{NRMCOL}          {NRMCOL}d0,_GfxBase");


    // Increase x to simulate scrolling of the whole page to the left by 9 chars
    textArea.m_X += 9;
    num = textArea.renderLine(82, false, lineTop, 0, renderBuf);
    BOOST_CHECK_EQUAL(num, 4);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}       {NRMCOL}move.l{NRMCOL}          {NRMCOL}d0,_GfxBase");

    // Increase x to simulate scrolling of the whole page to the left for another char
    textArea.m_X += 1;
    num = textArea.renderLine(82, false, lineTop, 0, renderBuf);
    BOOST_CHECK_EQUAL(num, 4);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}      {NRMCOL}move.l{NRMCOL}          {NRMCOL}d0,_GfxBase");

    // Increase x to simulate scrolling of the whole page to the left for another char
    textArea.m_X += 1;
    num = textArea.renderLine(82, false, lineTop, 0, renderBuf);
    BOOST_CHECK_EQUAL(num, 4);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}     {NRMCOL}move.l{NRMCOL}          {NRMCOL}d0,_GfxBase");

    textArea.m_X += 1;
    num = textArea.renderLine(82, false, lineTop, 0, renderBuf);
    BOOST_CHECK_EQUAL(num, 4);
    BOOST_CHECK_EQUAL(renderBuf, "{NRMCOL}    {NRMCOL}move.l{NRMCOL}          {NRMCOL}d0,_GfxBase");
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}



/**
 * testcase_36
 *
 * Test the IGNORETRAILINGSPACES parameter. If it is set, comparing the
 * both files of testcase_36 should result in no differences.
 */
BOOST_AUTO_TEST_CASE( test_36_IgnoreTrailingSpaces )
{
  try
  {
    bool ignoreLeadingSpaces = false;
    bool ignoreTrailingSpaces = true;
    bool lineNumbersEnabled = true;
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_36_trailing_spaces_left.txt",
                            lineNumbersEnabled,
                            ignoreLeadingSpaces,
                            ignoreTrailingSpaces);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_36_trailing_spaces_right.txt",
                            lineNumbersEnabled,
                            ignoreLeadingSpaces,
                            ignoreTrailingSpaces);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    // Should be 0 when IGNORETRAILINGSPACES set 
    long numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 1);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


/**
 * testcase_37
 *
 * Test the IGNORELEADINGSPACES parameter. If it is set, comparing the
 * both files of testcase_37 should result in just one difference.
 */
BOOST_AUTO_TEST_CASE( test_37_IgnoreLeadingSpaces )
{
  try
  {
    bool ignoreLeadingSpaces = true;
    bool ignoreTrailingSpaces = false;
    bool lineNumbersEnabled = true;
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_37_leading_spaces_left.c",
                            lineNumbersEnabled,
                            ignoreLeadingSpaces,
                            ignoreTrailingSpaces);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_37_leading_spaces_right.c",
                            lineNumbersEnabled,
                            ignoreLeadingSpaces,
                            ignoreTrailingSpaces);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);
    diffEngine.startCompare();

    // Should be 1 when IGNORELEADINGSPACES set 
    long numDifferences = diffEngine.getNumDifferences();
    BOOST_CHECK_EQUAL(numDifferences, 1);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}


BOOST_AUTO_TEST_CASE( testcase_38_combined_added_lines )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "testfiles/testcase_38_combined_added_lines_left.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "testfiles/testcase_38_combined_added_lines_right.txt",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices, true);
    diffEngine.startCompare();

    BOOST_CHECK_EQUAL(diffEngine.getNumDifferences(), 7);
    BOOST_CHECK_EQUAL(diffEngine.getNumAdded(), 7);
    BOOST_CHECK_EQUAL(diffEngine.getNumDeleted(), 0);
    BOOST_CHECK_EQUAL(diffEngine.getNumChanged(), 0);
  }
  catch(const char* pError)
  {
    auto locationBoost = boost::unit_test::framework::current_test_case().p_name;
    std::string location(locationBoost);
    printf("Exception in test %s: %s\n", 
           location.c_str(),
           pError);

    // To let the test fail
    BOOST_CHECK_EQUAL(1, 2);
  }
}

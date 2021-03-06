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

#include <string>
#include <list>

#include "DiffInputFileLinux.h"
#include "DiffOutputFileLinux.h"
#include "DiffEngine.h"
#include "DiffLine.h"
#include "ProgressReporter.h"

#include "SelectableDiffFile.h"
#include "TextSelection.h"
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

    // printf("Left file:\n");
    // printFile(diffA);
    // printf("\nRight file:\n");
    // printFile(diffB);

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
 * See Excel sheet Excel sheet 
 *   'ADiffView - selected text and scrolling.xlsx
 * for a visual of this test.
 */
BOOST_AUTO_TEST_CASE( test_TextSelection )
{
  try
  {
    TextSelection selection;
    
    selection.add(1, 4, 14);
    selection.add(2, 0, 9);
    selection.add(3, 0, 3);

    selection.add(5, 3, 5);
    selection.add(5, 10, 13);


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

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 2);
    BOOST_CHECK_EQUAL(diffB.getNumLines(), 2);

    SelectableDiffFile diffASelectable(diffA);
    diffASelectable.addSelection(0, 7, 14);
    diffASelectable.addSelection(0, 26, 28);

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



BOOST_AUTO_TEST_CASE( testcase_crash )
{
  try
  {
    bool cancelRequested = false;
    std::list<size_t> m_DiffIndices;

    DiffInputFileLinux srcA(cancelRequested, 
                            "CMakeLists.txt",
                            true);

    DiffInputFileLinux srcB(cancelRequested, 
                            "LICENSE-3RD-PARTY",
                            true);

    DiffOutputFileLinux diffA(srcA);
    DiffOutputFileLinux diffB(srcB);
    DiffEngine diffEngine(srcA, srcB, diffA, diffB, progress,
                          "Comparing...", cancelRequested, m_DiffIndices);

    // printf("Left file:\n");
    // printFile(diffA);
    // printf("\nRight file:\n");
    // printFile(diffB);

    BOOST_CHECK_EQUAL(diffEngine.getNumDifferences(), 7);
    BOOST_CHECK_EQUAL(diffEngine.getNumAdded(), 5);
    BOOST_CHECK_EQUAL(diffEngine.getNumDeleted(), 0);
    BOOST_CHECK_EQUAL(diffEngine.getNumChanged(), 2);

    BOOST_CHECK_EQUAL(diffA.getNumLines(), 36);
    // BOOST_CHECK_EQUAL(diffA[0]->getText(), "");
    // BOOST_CHECK_EQUAL(diffA[0]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffA[0]->getLineNumText(), "");
    // BOOST_CHECK_EQUAL(diffA[1]->getText(), "cmake_minimum_required (VERSION 2.8.11)");
    // BOOST_CHECK_EQUAL(diffA[1]->getState(), DiffLine::Changed);
    // BOOST_CHECK_EQUAL(diffA[1]->getLineNumText(), "2 ");

    
    // BOOST_CHECK_EQUAL(diffA[2]->getText(), "");
    // BOOST_CHECK_EQUAL(diffA[2]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffA[2]->getLineNumText(), "");
    // BOOST_CHECK_EQUAL(diffA[3]->getText(), "CCCC");
    // BOOST_CHECK_EQUAL(diffA[3]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffA[3]->getLineNumText(), "3 ");
    // BOOST_CHECK_EQUAL(diffA[4]->getText(), "DDDD");
    // BOOST_CHECK_EQUAL(diffA[4]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffA[4]->getLineNumText(), "4 ");
    // BOOST_CHECK_EQUAL(diffA[5]->getText(), "EEEE");
    // BOOST_CHECK_EQUAL(diffA[5]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffA[5]->getLineNumText(), "5 ");
    // BOOST_CHECK_EQUAL(diffA[6]->getText(), "FFFF");
    // BOOST_CHECK_EQUAL(diffA[6]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffA[6]->getLineNumText(), "6 ");
    // BOOST_CHECK_EQUAL(diffA[7]->getText(), "GGGG");
    // BOOST_CHECK_EQUAL(diffA[7]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffA[7]->getLineNumText(), "7 ");

    BOOST_CHECK_EQUAL(diffB.getNumLines(), 36);
    // BOOST_CHECK_EQUAL(diffB[0]->getText(), "AAAA");
    // BOOST_CHECK_EQUAL(diffB[0]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffB[0]->getLineNumText(), "1 ");
    // BOOST_CHECK_EQUAL(diffB[1]->getText(), "FFFF");
    // BOOST_CHECK_EQUAL(diffB[1]->getState(), DiffLine::Changed);
    // BOOST_CHECK_EQUAL(diffB[1]->getLineNumText(), "2 ");
    // BOOST_CHECK_EQUAL(diffB[2]->getText(), "ffff");
    // BOOST_CHECK_EQUAL(diffB[2]->getState(), DiffLine::Added);
    // BOOST_CHECK_EQUAL(diffB[2]->getLineNumText(), "3 ");
    // BOOST_CHECK_EQUAL(diffB[3]->getText(), "CCCC");
    // BOOST_CHECK_EQUAL(diffB[3]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffB[3]->getLineNumText(), "4 ");
    // BOOST_CHECK_EQUAL(diffB[4]->getText(), "DDDD");
    // BOOST_CHECK_EQUAL(diffB[4]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffB[4]->getLineNumText(), "5 ");
    // BOOST_CHECK_EQUAL(diffB[5]->getText(), "EEEE");
    // BOOST_CHECK_EQUAL(diffB[5]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffB[5]->getLineNumText(), "6 ");
    // BOOST_CHECK_EQUAL(diffB[6]->getText(), "FFFF");
    // BOOST_CHECK_EQUAL(diffB[6]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffB[6]->getLineNumText(), "7 ");
    // BOOST_CHECK_EQUAL(diffB[7]->getText(), "GGGG");
    // BOOST_CHECK_EQUAL(diffB[7]->getState(), DiffLine::Normal);
    // BOOST_CHECK_EQUAL(diffB[7]->getLineNumText(), "8 ");
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

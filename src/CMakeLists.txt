 
cmake_minimum_required (VERSION 2.8.11)

set(CROSS_COMPILE m68k-amigaos-)

SET(CMAKE_CXX_COMPILER m68k-amigaos-g++)

SET(CMAKE_CXX_FLAGS "-noixemul -Wall -O2 -s")

set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <OBJECTS> -o <TARGET>")

set(default_build_type "Release")

add_executable (ADiffView_gcc
                amigautils/AmigaFile.cpp
                amigautils/MemoryPool.cpp
                amigautils/MessageBox.cpp
                amigautils/StopWatch.cpp
                application/ADiffViewArgs.cpp
                application/ADiffViewPens.cpp
                application/ADiffViewPorts.cpp
                application/ADiffViewSettings.cpp
                application/Application.cpp
                application/DiffDocument.cpp
                application/DiffWorker.cpp
                application/DiffWorkerProgressReporter.cpp
                application/TextFinder.cpp
                commands/CmdChangeTabulatorWidth.cpp
                commands/CmdCloseWindow.cpp
                commands/CmdDiff.cpp
                commands/CmdFindNext.cpp
                commands/CmdFindPrev.cpp
                commands/CmdFileRequester.cpp
                commands/CmdNavigateBase.cpp
                commands/CmdNavigateFirstDiff.cpp
                commands/CmdNavigatePrevDiff.cpp
                commands/CmdNavigateNextDiff.cpp
                commands/CmdNavigateLastDiff.cpp
                commands/CmdOpenWindow.cpp
                commands/CmdQuit.cpp
                commands/CmdRequester.cpp
                commands/CmdWindowTitleQuickMsg.cpp
                commands/CommandBase.cpp
                diffengine/DiffEngine.cpp
                diffengine/DiffFileBase.cpp
                diffengine/DiffInputFileAmiga.cpp
                diffengine/DiffInputFileBase.cpp
                diffengine/DiffOutputFileAmiga.cpp
                diffengine/DiffOutputFileBase.cpp
                diffengine/DiffLine.cpp
                diffengine/SelectableDiffFile.cpp
                menus/MenuBase.cpp
                menus/MenuFilesWindow.cpp
                menus/MenuDiffWindow.cpp
                model/Pair.cpp
                model/Rect.cpp
                screen/OpenClonedWorkbenchScreen.cpp
                screen/OpenJoinedPublicScreen.cpp
                screen/OpenScreenBase.cpp
                textsearch/DiffFileSearchEngine.cpp
                textsearch/DiffFileSearchResult.cpp
                textselect/TextSelection.cpp
                textselect/TextSelectionRange.cpp
                textselect/TextSelectionLine.cpp
                windows/DiffWindow.cpp
                windows/DiffWindowRastports.cpp
                windows/DiffWindowTextArea.cpp
                windows/FilesWindow.cpp
                windows/ProgressWindow.cpp
                windows/SearchWindow.cpp
                windows/ScrollbarWindow.cpp
                windows/WindowBase.cpp
                worker/ProgressReporter.cpp
                worker/WorkerBase.cpp
                main.cpp)


target_include_directories (ADiffView_gcc PUBLIC
                            amigautils
                            application
                            commands
                            diffengine
                            menus
                            model
                            screen
                            textsearch
                            textselect
                            windows
                            worker
                            .)

add_custom_command(TARGET ADiffView_gcc
                   POST_BUILD
                   COMMAND cp ADiffView_gcc ${CMAKE_CURRENT_SOURCE_DIR}/..
                   DEPENDS ADiffView_gcc)

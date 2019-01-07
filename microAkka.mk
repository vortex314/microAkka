##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=microAkka
ConfigurationName      :=Debug
WorkspacePath          :=/home/lieven/workspace
ProjectPath            :=/home/lieven/workspace/microAkka
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Lieven
Date                   :=07/01/19
CodeLitePath           :=/home/lieven/.codelite
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="microAkka.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -pthread -lrt
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)src $(IncludeSwitch)../Common $(IncludeSwitch)../paho.mqtt.c/src $(IncludeSwitch)../ArduinoJson/src $(IncludeSwitch)../FreeRTOS/Source/include $(IncludeSwitch)../freertos-addons/Linux/portable/GCC/Linux 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)Common $(LibrarySwitch)paho-mqtt3a-static 
ArLibs                 :=  "Common" "libpaho-mqtt3a-static" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../Common/Debug $(LibraryPathSwitch)../paho.mqtt.c/src 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -O0 -std=c++11 -Wall -DLINUX_PORT_DEBUG $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall -DLINUX_PORT_DEBUG $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/src_Akka.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_NeuralPid.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Metric.cpp$(ObjectSuffix) $(IntermediateDirectory)/FreeRTOS_stream_buffer.c$(ObjectSuffix) $(IntermediateDirectory)/FreeRTOS_list.c$(ObjectSuffix) $(IntermediateDirectory)/FreeRTOS_timers.c$(ObjectSuffix) $(IntermediateDirectory)/FreeRTOS_portable_MemMang_heap_3.c$(ObjectSuffix) \
	$(IntermediateDirectory)/src_System.cpp$(ObjectSuffix) $(IntermediateDirectory)/FreeRTOS_croutine.c$(ObjectSuffix) $(IntermediateDirectory)/FreeRTOS_tasks.c$(ObjectSuffix) $(IntermediateDirectory)/FreeRTOS_event_groups.c$(ObjectSuffix) $(IntermediateDirectory)/src_ConfigActor.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Publisher.cpp$(ObjectSuffix) $(IntermediateDirectory)/Linux_port.c$(ObjectSuffix) $(IntermediateDirectory)/src_Mqtt.cpp$(ObjectSuffix) $(IntermediateDirectory)/FreeRTOS_queue.c$(ObjectSuffix) \
	$(IntermediateDirectory)/src_Bridge.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d "../.build-debug/Common" $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

"../.build-debug/Common":
	@$(MakeDirCommand) "../.build-debug"
	@echo stam > "../.build-debug/Common"




MakeIntermediateDirs:
	@test -d ./Debug || $(MakeDirCommand) ./Debug


$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_Akka.cpp$(ObjectSuffix): src/Akka.cpp $(IntermediateDirectory)/src_Akka.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Akka.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Akka.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Akka.cpp$(DependSuffix): src/Akka.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Akka.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Akka.cpp$(DependSuffix) -MM src/Akka.cpp

$(IntermediateDirectory)/src_Akka.cpp$(PreprocessSuffix): src/Akka.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Akka.cpp$(PreprocessSuffix) src/Akka.cpp

$(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix): src/Echo.cpp $(IntermediateDirectory)/src_Echo.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Echo.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Echo.cpp$(DependSuffix): src/Echo.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Echo.cpp$(DependSuffix) -MM src/Echo.cpp

$(IntermediateDirectory)/src_Echo.cpp$(PreprocessSuffix): src/Echo.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Echo.cpp$(PreprocessSuffix) src/Echo.cpp

$(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix): src/Sender.cpp $(IntermediateDirectory)/src_Sender.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Sender.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Sender.cpp$(DependSuffix): src/Sender.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Sender.cpp$(DependSuffix) -MM src/Sender.cpp

$(IntermediateDirectory)/src_Sender.cpp$(PreprocessSuffix): src/Sender.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Sender.cpp$(PreprocessSuffix) src/Sender.cpp

$(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix): src/Machinelearning.cpp $(IntermediateDirectory)/src_Machinelearning.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Machinelearning.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Machinelearning.cpp$(DependSuffix): src/Machinelearning.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Machinelearning.cpp$(DependSuffix) -MM src/Machinelearning.cpp

$(IntermediateDirectory)/src_Machinelearning.cpp$(PreprocessSuffix): src/Machinelearning.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Machinelearning.cpp$(PreprocessSuffix) src/Machinelearning.cpp

$(IntermediateDirectory)/src_NeuralPid.cpp$(ObjectSuffix): src/NeuralPid.cpp $(IntermediateDirectory)/src_NeuralPid.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/NeuralPid.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_NeuralPid.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_NeuralPid.cpp$(DependSuffix): src/NeuralPid.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_NeuralPid.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_NeuralPid.cpp$(DependSuffix) -MM src/NeuralPid.cpp

$(IntermediateDirectory)/src_NeuralPid.cpp$(PreprocessSuffix): src/NeuralPid.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_NeuralPid.cpp$(PreprocessSuffix) src/NeuralPid.cpp

$(IntermediateDirectory)/src_Metric.cpp$(ObjectSuffix): src/Metric.cpp $(IntermediateDirectory)/src_Metric.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Metric.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Metric.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Metric.cpp$(DependSuffix): src/Metric.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Metric.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Metric.cpp$(DependSuffix) -MM src/Metric.cpp

$(IntermediateDirectory)/src_Metric.cpp$(PreprocessSuffix): src/Metric.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Metric.cpp$(PreprocessSuffix) src/Metric.cpp

$(IntermediateDirectory)/FreeRTOS_stream_buffer.c$(ObjectSuffix): FreeRTOS/stream_buffer.c $(IntermediateDirectory)/FreeRTOS_stream_buffer.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/lieven/workspace/microAkka/FreeRTOS/stream_buffer.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/FreeRTOS_stream_buffer.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/FreeRTOS_stream_buffer.c$(DependSuffix): FreeRTOS/stream_buffer.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/FreeRTOS_stream_buffer.c$(ObjectSuffix) -MF$(IntermediateDirectory)/FreeRTOS_stream_buffer.c$(DependSuffix) -MM FreeRTOS/stream_buffer.c

$(IntermediateDirectory)/FreeRTOS_stream_buffer.c$(PreprocessSuffix): FreeRTOS/stream_buffer.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/FreeRTOS_stream_buffer.c$(PreprocessSuffix) FreeRTOS/stream_buffer.c

$(IntermediateDirectory)/FreeRTOS_list.c$(ObjectSuffix): FreeRTOS/list.c $(IntermediateDirectory)/FreeRTOS_list.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/lieven/workspace/microAkka/FreeRTOS/list.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/FreeRTOS_list.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/FreeRTOS_list.c$(DependSuffix): FreeRTOS/list.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/FreeRTOS_list.c$(ObjectSuffix) -MF$(IntermediateDirectory)/FreeRTOS_list.c$(DependSuffix) -MM FreeRTOS/list.c

$(IntermediateDirectory)/FreeRTOS_list.c$(PreprocessSuffix): FreeRTOS/list.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/FreeRTOS_list.c$(PreprocessSuffix) FreeRTOS/list.c

$(IntermediateDirectory)/FreeRTOS_timers.c$(ObjectSuffix): FreeRTOS/timers.c $(IntermediateDirectory)/FreeRTOS_timers.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/lieven/workspace/microAkka/FreeRTOS/timers.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/FreeRTOS_timers.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/FreeRTOS_timers.c$(DependSuffix): FreeRTOS/timers.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/FreeRTOS_timers.c$(ObjectSuffix) -MF$(IntermediateDirectory)/FreeRTOS_timers.c$(DependSuffix) -MM FreeRTOS/timers.c

$(IntermediateDirectory)/FreeRTOS_timers.c$(PreprocessSuffix): FreeRTOS/timers.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/FreeRTOS_timers.c$(PreprocessSuffix) FreeRTOS/timers.c

$(IntermediateDirectory)/FreeRTOS_portable_MemMang_heap_3.c$(ObjectSuffix): FreeRTOS/portable/MemMang/heap_3.c $(IntermediateDirectory)/FreeRTOS_portable_MemMang_heap_3.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/lieven/workspace/microAkka/FreeRTOS/portable/MemMang/heap_3.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/FreeRTOS_portable_MemMang_heap_3.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/FreeRTOS_portable_MemMang_heap_3.c$(DependSuffix): FreeRTOS/portable/MemMang/heap_3.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/FreeRTOS_portable_MemMang_heap_3.c$(ObjectSuffix) -MF$(IntermediateDirectory)/FreeRTOS_portable_MemMang_heap_3.c$(DependSuffix) -MM FreeRTOS/portable/MemMang/heap_3.c

$(IntermediateDirectory)/FreeRTOS_portable_MemMang_heap_3.c$(PreprocessSuffix): FreeRTOS/portable/MemMang/heap_3.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/FreeRTOS_portable_MemMang_heap_3.c$(PreprocessSuffix) FreeRTOS/portable/MemMang/heap_3.c

$(IntermediateDirectory)/src_System.cpp$(ObjectSuffix): src/System.cpp $(IntermediateDirectory)/src_System.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/System.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_System.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_System.cpp$(DependSuffix): src/System.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_System.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_System.cpp$(DependSuffix) -MM src/System.cpp

$(IntermediateDirectory)/src_System.cpp$(PreprocessSuffix): src/System.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_System.cpp$(PreprocessSuffix) src/System.cpp

$(IntermediateDirectory)/FreeRTOS_croutine.c$(ObjectSuffix): FreeRTOS/croutine.c $(IntermediateDirectory)/FreeRTOS_croutine.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/lieven/workspace/microAkka/FreeRTOS/croutine.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/FreeRTOS_croutine.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/FreeRTOS_croutine.c$(DependSuffix): FreeRTOS/croutine.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/FreeRTOS_croutine.c$(ObjectSuffix) -MF$(IntermediateDirectory)/FreeRTOS_croutine.c$(DependSuffix) -MM FreeRTOS/croutine.c

$(IntermediateDirectory)/FreeRTOS_croutine.c$(PreprocessSuffix): FreeRTOS/croutine.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/FreeRTOS_croutine.c$(PreprocessSuffix) FreeRTOS/croutine.c

$(IntermediateDirectory)/FreeRTOS_tasks.c$(ObjectSuffix): FreeRTOS/tasks.c $(IntermediateDirectory)/FreeRTOS_tasks.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/lieven/workspace/microAkka/FreeRTOS/tasks.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/FreeRTOS_tasks.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/FreeRTOS_tasks.c$(DependSuffix): FreeRTOS/tasks.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/FreeRTOS_tasks.c$(ObjectSuffix) -MF$(IntermediateDirectory)/FreeRTOS_tasks.c$(DependSuffix) -MM FreeRTOS/tasks.c

$(IntermediateDirectory)/FreeRTOS_tasks.c$(PreprocessSuffix): FreeRTOS/tasks.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/FreeRTOS_tasks.c$(PreprocessSuffix) FreeRTOS/tasks.c

$(IntermediateDirectory)/FreeRTOS_event_groups.c$(ObjectSuffix): FreeRTOS/event_groups.c $(IntermediateDirectory)/FreeRTOS_event_groups.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/lieven/workspace/microAkka/FreeRTOS/event_groups.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/FreeRTOS_event_groups.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/FreeRTOS_event_groups.c$(DependSuffix): FreeRTOS/event_groups.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/FreeRTOS_event_groups.c$(ObjectSuffix) -MF$(IntermediateDirectory)/FreeRTOS_event_groups.c$(DependSuffix) -MM FreeRTOS/event_groups.c

$(IntermediateDirectory)/FreeRTOS_event_groups.c$(PreprocessSuffix): FreeRTOS/event_groups.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/FreeRTOS_event_groups.c$(PreprocessSuffix) FreeRTOS/event_groups.c

$(IntermediateDirectory)/src_ConfigActor.cpp$(ObjectSuffix): src/ConfigActor.cpp $(IntermediateDirectory)/src_ConfigActor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/ConfigActor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ConfigActor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ConfigActor.cpp$(DependSuffix): src/ConfigActor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ConfigActor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ConfigActor.cpp$(DependSuffix) -MM src/ConfigActor.cpp

$(IntermediateDirectory)/src_ConfigActor.cpp$(PreprocessSuffix): src/ConfigActor.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ConfigActor.cpp$(PreprocessSuffix) src/ConfigActor.cpp

$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix): src/main.cpp $(IntermediateDirectory)/src_main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_main.cpp$(DependSuffix): src/main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_main.cpp$(DependSuffix) -MM src/main.cpp

$(IntermediateDirectory)/src_main.cpp$(PreprocessSuffix): src/main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_main.cpp$(PreprocessSuffix) src/main.cpp

$(IntermediateDirectory)/src_Publisher.cpp$(ObjectSuffix): src/Publisher.cpp $(IntermediateDirectory)/src_Publisher.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Publisher.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Publisher.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Publisher.cpp$(DependSuffix): src/Publisher.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Publisher.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Publisher.cpp$(DependSuffix) -MM src/Publisher.cpp

$(IntermediateDirectory)/src_Publisher.cpp$(PreprocessSuffix): src/Publisher.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Publisher.cpp$(PreprocessSuffix) src/Publisher.cpp

$(IntermediateDirectory)/Linux_port.c$(ObjectSuffix): Linux/port.c $(IntermediateDirectory)/Linux_port.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/lieven/workspace/microAkka/Linux/port.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Linux_port.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Linux_port.c$(DependSuffix): Linux/port.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Linux_port.c$(ObjectSuffix) -MF$(IntermediateDirectory)/Linux_port.c$(DependSuffix) -MM Linux/port.c

$(IntermediateDirectory)/Linux_port.c$(PreprocessSuffix): Linux/port.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Linux_port.c$(PreprocessSuffix) Linux/port.c

$(IntermediateDirectory)/src_Mqtt.cpp$(ObjectSuffix): src/Mqtt.cpp $(IntermediateDirectory)/src_Mqtt.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Mqtt.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Mqtt.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Mqtt.cpp$(DependSuffix): src/Mqtt.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Mqtt.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Mqtt.cpp$(DependSuffix) -MM src/Mqtt.cpp

$(IntermediateDirectory)/src_Mqtt.cpp$(PreprocessSuffix): src/Mqtt.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Mqtt.cpp$(PreprocessSuffix) src/Mqtt.cpp

$(IntermediateDirectory)/FreeRTOS_queue.c$(ObjectSuffix): FreeRTOS/queue.c $(IntermediateDirectory)/FreeRTOS_queue.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/lieven/workspace/microAkka/FreeRTOS/queue.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/FreeRTOS_queue.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/FreeRTOS_queue.c$(DependSuffix): FreeRTOS/queue.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/FreeRTOS_queue.c$(ObjectSuffix) -MF$(IntermediateDirectory)/FreeRTOS_queue.c$(DependSuffix) -MM FreeRTOS/queue.c

$(IntermediateDirectory)/FreeRTOS_queue.c$(PreprocessSuffix): FreeRTOS/queue.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/FreeRTOS_queue.c$(PreprocessSuffix) FreeRTOS/queue.c

$(IntermediateDirectory)/src_Bridge.cpp$(ObjectSuffix): src/Bridge.cpp $(IntermediateDirectory)/src_Bridge.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Bridge.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Bridge.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Bridge.cpp$(DependSuffix): src/Bridge.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Bridge.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Bridge.cpp$(DependSuffix) -MM src/Bridge.cpp

$(IntermediateDirectory)/src_Bridge.cpp$(PreprocessSuffix): src/Bridge.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Bridge.cpp$(PreprocessSuffix) src/Bridge.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/



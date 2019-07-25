##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=microAkka
ConfigurationName      :=Debug
WorkspacePath          :=/Users/lieven/workspace
ProjectPath            :=/Users/lieven/workspace/microAkka
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Lieven Merckx
Date                   :=25/07/2019
CodeLitePath           :="/Users/lieven/Library/Application Support/CodeLite"
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -dynamiclib -fPIC
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
LinkOptions            :=  ../paho.mqtt.c/build/output/libpaho-mqtt3c.a
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)src $(IncludeSwitch)../Common $(IncludeSwitch)../paho.mqtt.c/src $(IncludeSwitch)../ArduinoJson/src $(IncludeSwitch)../FreeRTOS/Source/include $(IncludeSwitch)../freertos-addons/Linux/portable/GCC/Linux 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)Common $(LibrarySwitch)pthread 
ArLibs                 :=  "Common" "pthread" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../Common/Debug 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -std=c++11 -Wall -DLINUX_PORT_DEBUG $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall -DLINUX_PORT_DEBUG $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/Applications/codelite.app/Contents/SharedSupport/
Objects0=$(IntermediateDirectory)/src_Akka.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Native.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Metric.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_WiringPi.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_System.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Mqtt.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_ConfigActor.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/src_Bridge.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_NeuralPid.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Hash.cpp$(ObjectSuffix) 



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
	$(LinkerName) $(OutputSwitch)$(OutputFile) $(Objects) $(LibPath) $(Libs) $(LinkOptions)

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
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/Akka.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Akka.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Akka.cpp$(DependSuffix): src/Akka.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Akka.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Akka.cpp$(DependSuffix) -MM src/Akka.cpp

$(IntermediateDirectory)/src_Akka.cpp$(PreprocessSuffix): src/Akka.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Akka.cpp$(PreprocessSuffix) src/Akka.cpp

$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix): src/main.cpp $(IntermediateDirectory)/src_main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_main.cpp$(DependSuffix): src/main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_main.cpp$(DependSuffix) -MM src/main.cpp

$(IntermediateDirectory)/src_main.cpp$(PreprocessSuffix): src/main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_main.cpp$(PreprocessSuffix) src/main.cpp

$(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix): src/Echo.cpp $(IntermediateDirectory)/src_Echo.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/Echo.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Echo.cpp$(DependSuffix): src/Echo.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Echo.cpp$(DependSuffix) -MM src/Echo.cpp

$(IntermediateDirectory)/src_Echo.cpp$(PreprocessSuffix): src/Echo.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Echo.cpp$(PreprocessSuffix) src/Echo.cpp

$(IntermediateDirectory)/src_Native.cpp$(ObjectSuffix): src/Native.cpp $(IntermediateDirectory)/src_Native.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/Native.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Native.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Native.cpp$(DependSuffix): src/Native.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Native.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Native.cpp$(DependSuffix) -MM src/Native.cpp

$(IntermediateDirectory)/src_Native.cpp$(PreprocessSuffix): src/Native.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Native.cpp$(PreprocessSuffix) src/Native.cpp

$(IntermediateDirectory)/src_Metric.cpp$(ObjectSuffix): src/Metric.cpp $(IntermediateDirectory)/src_Metric.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/Metric.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Metric.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Metric.cpp$(DependSuffix): src/Metric.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Metric.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Metric.cpp$(DependSuffix) -MM src/Metric.cpp

$(IntermediateDirectory)/src_Metric.cpp$(PreprocessSuffix): src/Metric.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Metric.cpp$(PreprocessSuffix) src/Metric.cpp

$(IntermediateDirectory)/src_WiringPi.cpp$(ObjectSuffix): src/WiringPi.cpp $(IntermediateDirectory)/src_WiringPi.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/WiringPi.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_WiringPi.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_WiringPi.cpp$(DependSuffix): src/WiringPi.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_WiringPi.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_WiringPi.cpp$(DependSuffix) -MM src/WiringPi.cpp

$(IntermediateDirectory)/src_WiringPi.cpp$(PreprocessSuffix): src/WiringPi.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_WiringPi.cpp$(PreprocessSuffix) src/WiringPi.cpp

$(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix): src/Machinelearning.cpp $(IntermediateDirectory)/src_Machinelearning.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/Machinelearning.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Machinelearning.cpp$(DependSuffix): src/Machinelearning.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Machinelearning.cpp$(DependSuffix) -MM src/Machinelearning.cpp

$(IntermediateDirectory)/src_Machinelearning.cpp$(PreprocessSuffix): src/Machinelearning.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Machinelearning.cpp$(PreprocessSuffix) src/Machinelearning.cpp

$(IntermediateDirectory)/src_System.cpp$(ObjectSuffix): src/System.cpp $(IntermediateDirectory)/src_System.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/System.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_System.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_System.cpp$(DependSuffix): src/System.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_System.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_System.cpp$(DependSuffix) -MM src/System.cpp

$(IntermediateDirectory)/src_System.cpp$(PreprocessSuffix): src/System.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_System.cpp$(PreprocessSuffix) src/System.cpp

$(IntermediateDirectory)/src_Mqtt.cpp$(ObjectSuffix): src/Mqtt.cpp $(IntermediateDirectory)/src_Mqtt.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/Mqtt.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Mqtt.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Mqtt.cpp$(DependSuffix): src/Mqtt.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Mqtt.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Mqtt.cpp$(DependSuffix) -MM src/Mqtt.cpp

$(IntermediateDirectory)/src_Mqtt.cpp$(PreprocessSuffix): src/Mqtt.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Mqtt.cpp$(PreprocessSuffix) src/Mqtt.cpp

$(IntermediateDirectory)/src_ConfigActor.cpp$(ObjectSuffix): src/ConfigActor.cpp $(IntermediateDirectory)/src_ConfigActor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/ConfigActor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_ConfigActor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_ConfigActor.cpp$(DependSuffix): src/ConfigActor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_ConfigActor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_ConfigActor.cpp$(DependSuffix) -MM src/ConfigActor.cpp

$(IntermediateDirectory)/src_ConfigActor.cpp$(PreprocessSuffix): src/ConfigActor.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_ConfigActor.cpp$(PreprocessSuffix) src/ConfigActor.cpp

$(IntermediateDirectory)/src_Bridge.cpp$(ObjectSuffix): src/Bridge.cpp $(IntermediateDirectory)/src_Bridge.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/Bridge.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Bridge.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Bridge.cpp$(DependSuffix): src/Bridge.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Bridge.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Bridge.cpp$(DependSuffix) -MM src/Bridge.cpp

$(IntermediateDirectory)/src_Bridge.cpp$(PreprocessSuffix): src/Bridge.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Bridge.cpp$(PreprocessSuffix) src/Bridge.cpp

$(IntermediateDirectory)/src_NeuralPid.cpp$(ObjectSuffix): src/NeuralPid.cpp $(IntermediateDirectory)/src_NeuralPid.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/NeuralPid.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_NeuralPid.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_NeuralPid.cpp$(DependSuffix): src/NeuralPid.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_NeuralPid.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_NeuralPid.cpp$(DependSuffix) -MM src/NeuralPid.cpp

$(IntermediateDirectory)/src_NeuralPid.cpp$(PreprocessSuffix): src/NeuralPid.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_NeuralPid.cpp$(PreprocessSuffix) src/NeuralPid.cpp

$(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix): src/Sender.cpp $(IntermediateDirectory)/src_Sender.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/Sender.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Sender.cpp$(DependSuffix): src/Sender.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Sender.cpp$(DependSuffix) -MM src/Sender.cpp

$(IntermediateDirectory)/src_Sender.cpp$(PreprocessSuffix): src/Sender.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Sender.cpp$(PreprocessSuffix) src/Sender.cpp

$(IntermediateDirectory)/src_Hash.cpp$(ObjectSuffix): src/Hash.cpp $(IntermediateDirectory)/src_Hash.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/lieven/workspace/microAkka/src/Hash.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Hash.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Hash.cpp$(DependSuffix): src/Hash.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Hash.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Hash.cpp$(DependSuffix) -MM src/Hash.cpp

$(IntermediateDirectory)/src_Hash.cpp$(PreprocessSuffix): src/Hash.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Hash.cpp$(PreprocessSuffix) src/Hash.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/



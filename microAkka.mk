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
Date                   :=15/12/18
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
LinkOptions            :=  -pthread
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)src $(IncludeSwitch)../Common $(IncludeSwitch)../paho.mqtt.c/src $(IncludeSwitch)../ArduinoJson/src 
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
CXXFLAGS :=  -g -O0 -Wall -fno-rtti -fno-exceptions -std=c++11 $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/src_Akka.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Linux.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Sys.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_NeuralPid.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Metric.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_System.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_MqttBridge.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) 



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

$(IntermediateDirectory)/src_Linux.cpp$(ObjectSuffix): src/Linux.cpp $(IntermediateDirectory)/src_Linux.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Linux.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Linux.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Linux.cpp$(DependSuffix): src/Linux.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Linux.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Linux.cpp$(DependSuffix) -MM src/Linux.cpp

$(IntermediateDirectory)/src_Linux.cpp$(PreprocessSuffix): src/Linux.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Linux.cpp$(PreprocessSuffix) src/Linux.cpp

$(IntermediateDirectory)/src_Sys.cpp$(ObjectSuffix): src/Sys.cpp $(IntermediateDirectory)/src_Sys.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Sys.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Sys.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Sys.cpp$(DependSuffix): src/Sys.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Sys.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Sys.cpp$(DependSuffix) -MM src/Sys.cpp

$(IntermediateDirectory)/src_Sys.cpp$(PreprocessSuffix): src/Sys.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Sys.cpp$(PreprocessSuffix) src/Sys.cpp

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

$(IntermediateDirectory)/src_System.cpp$(ObjectSuffix): src/System.cpp $(IntermediateDirectory)/src_System.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/System.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_System.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_System.cpp$(DependSuffix): src/System.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_System.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_System.cpp$(DependSuffix) -MM src/System.cpp

$(IntermediateDirectory)/src_System.cpp$(PreprocessSuffix): src/System.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_System.cpp$(PreprocessSuffix) src/System.cpp

$(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix): src/Machinelearning.cpp $(IntermediateDirectory)/src_Machinelearning.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Machinelearning.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Machinelearning.cpp$(DependSuffix): src/Machinelearning.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Machinelearning.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Machinelearning.cpp$(DependSuffix) -MM src/Machinelearning.cpp

$(IntermediateDirectory)/src_Machinelearning.cpp$(PreprocessSuffix): src/Machinelearning.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Machinelearning.cpp$(PreprocessSuffix) src/Machinelearning.cpp

$(IntermediateDirectory)/src_MqttBridge.cpp$(ObjectSuffix): src/MqttBridge.cpp $(IntermediateDirectory)/src_MqttBridge.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/MqttBridge.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_MqttBridge.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_MqttBridge.cpp$(DependSuffix): src/MqttBridge.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_MqttBridge.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_MqttBridge.cpp$(DependSuffix) -MM src/MqttBridge.cpp

$(IntermediateDirectory)/src_MqttBridge.cpp$(PreprocessSuffix): src/MqttBridge.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_MqttBridge.cpp$(PreprocessSuffix) src/MqttBridge.cpp

$(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix): src/Sender.cpp $(IntermediateDirectory)/src_Sender.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Sender.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Sender.cpp$(DependSuffix): src/Sender.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Sender.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Sender.cpp$(DependSuffix) -MM src/Sender.cpp

$(IntermediateDirectory)/src_Sender.cpp$(PreprocessSuffix): src/Sender.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Sender.cpp$(PreprocessSuffix) src/Sender.cpp

$(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix): src/Echo.cpp $(IntermediateDirectory)/src_Echo.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Echo.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Echo.cpp$(DependSuffix): src/Echo.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Echo.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Echo.cpp$(DependSuffix) -MM src/Echo.cpp

$(IntermediateDirectory)/src_Echo.cpp$(PreprocessSuffix): src/Echo.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Echo.cpp$(PreprocessSuffix) src/Echo.cpp

$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix): src/main.cpp $(IntermediateDirectory)/src_main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_main.cpp$(DependSuffix): src/main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_main.cpp$(DependSuffix) -MM src/main.cpp

$(IntermediateDirectory)/src_main.cpp$(PreprocessSuffix): src/main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_main.cpp$(PreprocessSuffix) src/main.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/



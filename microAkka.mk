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
Date                   :=25/08/18
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
CXXFLAGS :=  -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/src_Akka.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Uid.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_MqttBridge.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Linux.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Sys.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

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

$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix): src/main.cpp $(IntermediateDirectory)/src_main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_main.cpp$(DependSuffix): src/main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_main.cpp$(DependSuffix) -MM src/main.cpp

$(IntermediateDirectory)/src_main.cpp$(PreprocessSuffix): src/main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_main.cpp$(PreprocessSuffix) src/main.cpp

$(IntermediateDirectory)/src_Uid.cpp$(ObjectSuffix): src/Uid.cpp $(IntermediateDirectory)/src_Uid.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/Uid.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Uid.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Uid.cpp$(DependSuffix): src/Uid.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Uid.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Uid.cpp$(DependSuffix) -MM src/Uid.cpp

$(IntermediateDirectory)/src_Uid.cpp$(PreprocessSuffix): src/Uid.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Uid.cpp$(PreprocessSuffix) src/Uid.cpp

$(IntermediateDirectory)/src_MqttBridge.cpp$(ObjectSuffix): src/MqttBridge.cpp $(IntermediateDirectory)/src_MqttBridge.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/lieven/workspace/microAkka/src/MqttBridge.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_MqttBridge.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_MqttBridge.cpp$(DependSuffix): src/MqttBridge.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_MqttBridge.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_MqttBridge.cpp$(DependSuffix) -MM src/MqttBridge.cpp

$(IntermediateDirectory)/src_MqttBridge.cpp$(PreprocessSuffix): src/MqttBridge.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_MqttBridge.cpp$(PreprocessSuffix) src/MqttBridge.cpp

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


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/



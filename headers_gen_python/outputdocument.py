from enum import Enum
class CanFrame:
    frameName = None
    dataType = []
    dataNames = []
    comments = []
    frequency = None
    id = None
class CustomEnum:
    name =None
    states = []
    comments = []
class CustomEnum:
    name = None
    states = []
    comments = []
class OutputDocument:
    __performIllegalCharsCheck = None
    __fileName = None
    __deviceName = None
    __deviceStateComments = []
    __deviceStates = []
    __verbatim = []
    __customEnumVec = []
    def __init__(self ,genFileName ,performCheck):
        self.__performIllegalCharsCheck = performCheck
        self.__fileName = genFileName
        self.__fileName = self.__fileName.replace('.csv' ,'.hpp')
        __file = open(self.__fileName, 'w')

    def removeIllegalChars(self ,target):
        strCopy =target
        if self.__performIllegalCharsCheck:
            for ch in range(len(strCopy)):
                if strCopy[ch] == ' ' or strCopy[ch] == '-':
                    strCopy = strCopy[0:ch] + '_' + strCopy[ch + 1::]
        return strCopy

    def setDeviceName(self, devName):
        self.__deviceName = self.removeIllegalChars(devName)

    def addDeviceState(self, newDeviceState):
        self.__deviceStates.append(newDeviceState)

    def addDeviceStateComment(self,devStateComment):
        self.__deviceStateComments.append(devStateComment)

    def addVerbatim(self, text):
        self.__verbatim.append(text)

    def addNewEnum(self,name):
        newEnum = CustomEnum()
        newEnum.name = name
        self.__customEnumVec.append(newEnum)

    def addEnumElement(self,name, comment):
        if len(self.__customEnumVec) == 0:
            raise Exception("Custom Enum vector is empty")

        lastEnum = self.__customEnumVec.back()
        lastEnum.states.append(name)
        lastEnum.comments.append(comment)
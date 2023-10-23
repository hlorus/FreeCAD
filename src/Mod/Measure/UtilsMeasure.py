from abc import ABC, abstractmethod, abstractclassmethod


class MeasureBasePython(ABC):

    @abstractclassmethod
    def isValidSelection(cls, selection):
        pass

    @abstractclassmethod
    def isPrioritySelection(cls, selection):
        pass

    @abstractmethod
    def parseSelection(self, selection):
        pass

    @abstractmethod
    def getResultString(self):
        pass


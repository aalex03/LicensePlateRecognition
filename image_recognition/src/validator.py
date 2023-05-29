
class LicensePlateValidator:
    counties=["AB","AR","AG","BC","BH","BN","BT","BR","BV","BZ","CL","CS","CJ","CT","CV","DB","DJ","GL","GR","GJ","HR","HD","IL","IS","IF","MM","MH","MS","NT","OT","PH","SJ","SM","SB","SV","TR","TM","TL","VL","VS","VN"]
    alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    digits = "0123456789"
    
    @staticmethod
    def _strspn(string : str, charSet : str) -> int:
        count=0
        for i in string:
            if i in charSet:
                count+=1
            else:
                break
        return count
    
    @staticmethod
    def _strcspn(string : str, charSet : str) -> int:
        count=0
        for i in string:
            if i not in charSet:
                count+=1
            else:
                break
        return count
    
    @staticmethod
    def validate(string : str) -> bool:
        valid = True
        judCount = LicensePlateValidator._strspn(string,LicensePlateValidator.alphabet)
        jud = string[:judCount]

        numCount = LicensePlateValidator._strspn(string[judCount:],LicensePlateValidator.digits)
        num = string[judCount:judCount+numCount]

        identificator = string[judCount+numCount:]
        # handle B
        if jud == "B":
            if(len(num) not in [2,3,6]):  # B 123456, B 12 ABC, B 123 ABC
                valid = False
            if(len(num) == 6 and len(identificator) != 0):  # B 123456 ABC
                valid = False
            if(len(num) !=6 and len(identificator) != 3):
                valid = False
        else:
            if(jud not in LicensePlateValidator.counties):
                valid = False
            if(len(num) not in [2,6]):
                valid = False
            if(len(num) == 6 and len(identificator) != 0):
                valid = False
            if(len(num) !=6 and len(identificator) != 3):
                valid = False
        return valid

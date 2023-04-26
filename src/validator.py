
class LicensePlateValidator:
    @staticmethod
    def validate(number : str):
        l=len(number.replace(" ",""))
        sep_number=number.split(" ",3)
        jud=["AB","AR","AG","BC","BH","BN","BT","BR","BV","BZ","CL","CS","CJ","CT","CV","DB","DJ","GL","GR","GJ","HR","HD","IL","IS","IF","MM","MH","MS","NT","OT","PH","SJ","SM","SB","SV","TR","TM","TL","VL","VS","VN","B"]
        if sep_number[0] in jud: #first sequence of characters is correct
            if len(sep_number[0])==2: #it is formed of 2 characters
                if len(sep_number[1])==2 and all(char.isdigit() for char in sep_number[1]) and len(sep_number[2])==3 and all(char.isalpha() for char in sep_number[2]) and l==7: #ex:AG 56 BAC
                    print("Identified number: "+ sep_number[0] + " " + sep_number[1] + " " + sep_number[2])
                elif len(sep_number[1])==6 and all(char.isdigit() for char in sep_number[1]) and l==8: #ex: CS 090234
                    print("Identified number: "+ sep_number[0] + " " + sep_number[1])
                else:
                        print("No Romanian number detected") 
            elif len(sep_number[0])==1: #it has only one character: B
                if len(sep_number[1])==6 and all(char.isdigit() for char in sep_number[1]) and l==7: #ex: B 030204
                    print("Identified number: "+ sep_number[0] + " " + sep_number[1])
                elif len(sep_number[1])==2 and all(char.isdigit() for char in sep_number[1]) and len(sep_number[2])==3 and all(char.isalpha() for char in sep_number[2]) and l==6: #ex: B 21 AAA
                    print("Identified number: "+ sep_number[0] + " " + sep_number[1]+" "+sep_number[2])
                elif len(sep_number[1])==3 and all(char.isdigit() for char in sep_number[1]) and len(sep_number[2])==3 and all(char.isalpha() for char in sep_number[2]) and l==7: #ex: B 322 BBB
                    print("Identified number: "+ sep_number[0] + " " + sep_number[1]+" "+sep_number[2])
                else:
                    print("No Romanian number detected")
        else:
            print("No Romanian number detected")       
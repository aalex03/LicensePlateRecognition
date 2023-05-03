
class LicensePlateValidator:
  @staticmethod
  def algo(number:str):
   number=number.replace(' ','')
   number=number.rstrip("\n")
   l=len(number)
   #sep_number=number.split(" ",3)
   jud=["AB","AR","AG","BC","BH","BN","BT","BR","BV","BZ","CL","CS","CJ","CT","CV","DB","DJ","GL","GR","GJ","HR","HD","IL","IS","IF","MM","MH","MS","NT","OT","PH","SJ","SM","SB","SV","TR","TM","TL","VL","VS","VN"]
   if(l>7):
    print("No Romanian number detected")  
   elif number[0]=='B' and number[1].isdigit() and number[2].isdigit() and all(char.isalpha() for char in number[3:]) and l==6:
      p1=number[0]
      p2=number[1:3]
      p3=number[3:7]
      print("Identified number: "+p1+" "+p2+" "+p3)
   elif number[0]=='B' and number[1].isdigit() and number[2].isdigit() and number[3].isdigit() and all(char.isalpha() for char in number[4:]) and l==7:
      p1=number[0]
      p2=number[1:4]
      p3=number[4:8]
      print("Identified number: "+p1+" "+p2+" "+p3)
   elif number[0]=='B' and all(char.isdigit() for char in number[1:]) and l==7:
      p1=number[0]
      p2=number[1:]
      print("Identified number: "+p1+" "+p2)
   elif number[0].isalpha() and number[1].isalpha() and all(char.isdigit() for char in number[2:4]) and all(char.isalpha() for char in number[4:7]) and l==7:
      p1=number[0:2]
      p2=number[2:4]
      p3=number[4:7]
      if p1 in jud:
        print("Identified number: "+p1+" "+p2+" "+p3)
      else:
        print("No Romanian number detected") 
   elif number[0].isalpha() and number[1].isalpha() and all(char.isdigit() for char in number[2:]) and l==8:
      p1=number[0:2]
      p2=number[2:]
      if p1 in jud:
        print("Identified number: "+p1+" "+p2)
      else:
        print("No Romanian number detected")
   else:
      print("No Romanian number detected")   
import random
import string

f = open("data.html", "w")
f.write("<html>\n")
f.write("<head>\n")
f.write("<title>The test page</title>\n")
f.write("<meta content=\"text/html;charset=utf-8\" http-equiv=\"Content-Type\">\n")
f.write("</head>\n")
f.write("<body>\n")
for i in range(0, 1000):
	ch = ''.join(random.choice(string.ascii_letters) for i in range(100))
	f.write("<p>" + ch + "</p>\n")
f.write("</body>\n")	
f.write("</html>\n")
f.close()

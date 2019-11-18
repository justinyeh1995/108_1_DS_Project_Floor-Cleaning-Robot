compare='/Users/mac/Desktop/Data_Structure_NTHU_108-1/Project_2/Testcase/1/floor.txt'#input file
fd=open(compare, 'r')
ksk=[]
y=0
for line_terminated in fd:#switch topic and its data by swtich line
    if(y==0):
        m=int(line_terminated.replace('\n','').split(" ")[0])
        n=int(line_terminated.replace('\n','').split(" ")[1])
        limit=int(line_terminated.replace('\n','').split(" ")[2])
        
    else:
        ksk.append( list(line_terminated.replace('\n','')) )
    y=1
fd.close() 
new_list = ksk[:]
for i in range(m):
    for j in range(n):
        if(ksk[i][j]=='R'):
            print("start",i,j)
            xx=i
            yy=j
            
target='/Users/mac/Desktop/Data_Structure_NTHU_108-1/Project_2/108_1_DS_Project_Floor-Cleaning-Robot/final.txt'#output file
fd=open(target, 'r')
kk=[]
y=0
for line_terminated in fd:#switch topic and its data by swtich line
    if(y==0):
        Total=int(line_terminated.replace('\n',''))
        y=1
    else:
        kk.append([int(line_terminated.replace('\n','').split(" ")[0]),int(line_terminated.replace('\n','').split(" ")[1])])
fd.close() 

ch=1
pp=0
for i,k in enumerate(kk):
    if(i>=2):
        ksk[k[0]][k[1]]='2'
        mm=abs(int(kk[i-1][0])-int(k[0]))+abs(int(kk[i-1][1])-int(k[1]))
        if(abs(mm)!=1):
                print("wrong",kk[i-1][0],kk[i-1][1],k[0],k[1])
        if(k[0]==xx and k[1]==yy):
            if((i-ch)>limit):
                print("exceed battery limit=",limit,"ch",i,"diff",i-ch)
            ch=i
            pp=pp+1
if(len(kk)-1!=Total):
    print("total=",len(kk)-1,Total)
for i in range(m):
    for j in range(n):
        if(new_list[i][j]==0):
            print("not fill ",i,j)
        if(new_list[i][j]==2 and ksk[i][j]==1):
            print("penetrate the wall",i,j)
rem =============================================
rem      Hit and Blow by Koji Iigura, 2022.
rem =============================================

print "=== HIT and BLOW ==="
input "input random seed: ",s
gosub 100 rem generate random number (u,v,w)

10 rem game loop
	gosub 400 rem user inout (a,b,c)
	gosub 500 rem calc H and B
	print "H=",h," B=",l
	if h=3 then goto 99
goto 10

99	print "Great answer!"
	print "YOU WIN!"
end

rem -----------------------------------
100 rem Random, answer is u,v,w
	x=s
110 rem retry
	for i=1 to 100
	    x=48271*x
	next
	x=x%1000
	if x<0 then x=-x
	z=x
	gosub 200 rem Decompose into individual digits
	gosub 300 rem Check constraint
	if r=1 then goto 190
	goto 110 rem retry!

190 rem found number
	print ">>> random value is ready."
	u=a v=b w=c
return

rem -----------------------------------
200 rem Decompose: z -> a,b,c
	a=(z/100)%10
	b=(z/10)%10
	c=z%10
return

rem -----------------------------------
300 rem Check constraint: a,b,c -> r
    rem if ok, r=1 otherwise r=0
	r=-1
	if a=b then r=0 return
	if b=c then r=0 return
	if c=a then r=0 return
	r=1
return

rem -----------------------------------
400 rem Get user input -> a,b,c
410 rem retry
	input "guess the number: ",y
	if y<0 then goto 999 rem Give Up
	z=y gosub 200 gosub 300
	if r=1 then return
	print "invalid number. guess again."
goto 410 rem retry

rem -----------------------------------
500 rem Calc H and B
	h=0	rem reset Hit
	if a=u then h=h+1
	if b=v then h=h+1
	if c=w then h=h+1

	l=0 rem reset Blow
	if a=v then l=l+1
	if a=w then l=l+1
	if b=u then l=l+1
	if b=w then l=l+1
	if c=u then l=l+1
	if c=v then l=l+1
return

999 rem Give Up!
	print "--- GAME OVER ---"
	print "the anser is ",x
	end

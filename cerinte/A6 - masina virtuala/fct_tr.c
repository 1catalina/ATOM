// pentru toate testele, urmatoarele declaratii sunt valabile

int max(int a,int b);
int max3(int a,int b,int c);
int random();
float sqrt(float x);
// constantele fara parte zecimala sunt considerate int: 108, 21
// constantele cu zecimale sunt considerate float: 3.14, 7.0

// test 1
float f(int n,float s){
	int i=0;
	while(i<n){
		if(i%2==0)s=s+i;
			else s=s-i;
		}
	return s;
	}
	
//test 2
int f(float v){
	float v1=random(), v2=random();
	if(v==max3(v,v1,v2))return 0;
	if(v1>v2)return -1;
	return 1;
	}
	
// test 3
int nSolutii(float a,float b,float c){
	float d2=b*b-4*a*c;
	int n;
	if(d2<0)n=0;
	else if(d2==0)n=1;
	else n=2;
	return n;
	}
	
// test 4
int fact(int n){
	if(n<0)return 0;
	int i=2,r=1;
	while(i<=n){
		r=r*i;
		i=i+1;
		}
	return r;
	}

// test 5
int ack(int m,int n){
	if(m==0)return n+1;
	else if(m>0&&n==0)return ack(m-1,1);
	return ack(m-1, ack(m,n-1));
	}

// test 6
float f(float a,float b,float c){
	float r=4*sqrt(a-b)/(c-b)-1;
	while(r>a)r=r/2;
	return r;
	}
	
// test 7
int cnt(int v,int x,int y){
	if(x>y)return -1;
	int n=0;
	while(x<y){
		n=n+1;
		y=y-x;
		}
	return n;
	}

// test 8
int nd(int v){
	int n=0;
	if(v<0)v=-v;
	while(v>0){
		v=v/10;
		n=n+1;
		}
	return n;
	}
	
// test 9
float f(float x,float y){
	if(x<y){
		while(x<0)x=x+7;
		}else{
		x=-y;
		}
	return x;
	}
	
// test 10
int g(int a,int b,float x){
	if(a<b){
		if(a<x&&x<b)return 1;
		}else{
		if(b<x&&x<a)return 1;
		}
	return 0;
	}

// test 11
float f(float x,float y,float z){
	float d=sqrt(x*x+y*y+z*z);
	if(d<10)return d/10+(x-y)/(z*3-1);
	return d/sqrt(x);
	}

// test 12
int nc(int a,int b,int c){
	int d1=a-b,d2=a-c;
	while(d1<d2){
		if(a!=d1)d1=d1/2;
		d2=a-d1;
		}
	return d1;
	}

// test 13
int ndiv(int n){
	int i=2,c=0;
	while(i<=n/2){
		if(n%i==0)c=c+1;
		i=i+1;
		}
	return c;
	}
	
// test 14
int rnd(int min,int max){
	int v;
	for(;;){
		v=random();
		if(v>=min&&v<=max)return v;
		}
	
	}

// test 15
int f(float x,float y){
	int n=0;
	while(x>y){
		x=x-y;
		n=n+1;
		if(n==100)return 100;
		}
	return n;
	}

// test 16
int h(int a,int b){
	int n;
	float f;
	while(a>b){
		f=a/(float)b;
		if(f==(int)f)n=n+1;
		b=b-1;
		}
	return n;
	}
	
// test 17
int f(int a,int b,int c){
	int n=0;
	while(max3(a,b,c)<100){
		a=a*2;
		c=a+b;
		n=n+1;
		}
	return n;
	}

// test 18
int h(float x,float y,float z){
		if(x<0){
			if(y==z)return 1;
			}else{
			if(y==z)return 0;
			}
		return x;
	}

// test 19
int f(int x){
	float r=x;
	int i=0;
	while(r>10){
		r=2*(r+0.5);
		i=i+1;
		}
	return i;
	}

// test 20
int h(float a,float b,float c){
	if((a+b)*(c-a)+1>c/(2+b))return 1;
	return sqrt(a+b+c);
	}
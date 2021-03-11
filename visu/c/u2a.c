#include <stdio.h>
//#include <conio.h>
#include <string.h>
#include <stdlib.h>
//#include <malloc.h>
#include "graphics.h"
#include "../opengl.h"
#include "../dis/dis.h"
#include "../cd.h"
#include "../c.h"

#define	noDEBUG
#define huge
#define _loadds

int	indemo=0;

char bg[320 * 200];
char *bg2;

char	scene[64]={"U2A"};
char	tmpname[68];

char huge *scene0;
char huge *scenem;

int	city=0;
int	xit=0;

#define LONGAT(zz) *((int huge *)(zz))
#define INTAT(zz) *((short huge *)(zz))
#define CHARAT(zz) *((char huge *)(zz))

struct s_scl
{
	char	*data;
} scenelist[64];
int scl=0,sclp=0;

#define MAXOBJ 256

struct s_co
{
	object	*o;
	long	dist;
	int	index;
	int	on;
} co[MAXOBJ];
int conum;

FILE	*fr;

object camobject;
rmatrix cam;

int	order[MAXOBJ],ordernum;
unsigned char huge *sp;

long lsget(unsigned char f)
{
	long	l;
	switch(f&3)
	{
	case 0 : l=0; 
		 break; 
	case 1 : l=(long)(char)(*sp++); 
		 break;
	case 2 : l=*sp++; 
		 l|=(long)(char)(*sp++)<<8; 
		 break;
	case 3 : l=*sp++;
		 l|=(long)(*sp++)<<8; 
		 l|=(long)(*sp++)<<16; 
		 l|=(long)(char)(*sp++)<<24; 
		 break;
	}
	return(l);
}		

void	resetscene(void)
{
	int	a;
	sp=(unsigned char *)(scenelist[sclp].data);
	for(a=0;a<conum;a++)
	{
		memset(co[a].o->r,0,sizeof(rmatrix));
		memset(co[a].o->r0,0,sizeof(rmatrix));
	}
	sclp++;
	if(sclp>=scl)
	{
		sclp=0;
	}
}

struct
{
	int	frames;
	int	ready; // 1=to be displayed, 0=displayed
} cl[4];
int	clr=0,clw=0;
int	deadlock=0;
int	coppercnt=0;
int	syncframe=0;
int	currframe=0;
int	copperdelay=16;
int	repeat,avgrepeat;

//#pragma check_stack(off)

void _loadds copper2(void)
{
	//int	a,c1,c2,c3,c4;
	
	syncframe++;

	if(cl[0].ready==2) cl[0].ready=0;
	if(cl[1].ready==2) cl[1].ready=0;
	if(cl[2].ready==2) cl[2].ready=0;
	if(cl[3].ready==2) cl[3].ready=0;

	deadlock++;
	coppercnt++;

	if(copperdelay>0)
	{
		copperdelay--;
	}
	if(copperdelay>0) return;
	copperdelay=0;
	if(cl[clr].ready)
	{
		cl[(clr-1)&3].ready=2;
		//vid_setswitch(-1,clr);
		copperdelay=cl[clr].frames;
		clr++; clr&=3;
	}
	else avgrepeat++;
}

int main(int argc,char *argv[])
{
	//char huge *sptemp;
	short	huge *ip;
	unsigned int u;
	char	huge *cp;
	int	a,b,c,d,e,f,g,x,y,z;
	#ifdef DEBUG
	//fr=fopen("tmp","wt");
	fr=stdout;
	#endif
	//indemo=1;

	dis_partstart();
	sprintf(tmpname,"%s.00M",scene);
	if(!indemo) printf("Loading materials %s...\n",tmpname);
	scene0=scenem=readfile(tmpname);

	memcpy(scene0+16+192*3,bg+16,64*3);
	bg2=calloc(16384,4);
	for(u=z=0;z<4;z++)
	{
		for(y=0;y<200;y++)
		{
			for(x=z;x<320;x+=4)
			{
				a=bg[16+768+x+y*320];
				bg2[u++]=a;
			}
		}
	}
	memcpy(bg,bg2,64000);
	free(bg2);

	if(scene0[15]=='C') city=1;
	if(scene0[15]=='R') city=2;
	ip=(short *)(scene0+LONGAT(scene0+4));
	conum=d=*ip++;
	for(f=-1,c=1;c<d;c++)
	{	
printf("c=%d\n", c);
		e=*ip++;
		if(e>f)
		{
			f=e;
			sprintf(tmpname,"%s.%03i",scene,e);
			if(!indemo) printf("Loading %s... ",tmpname);
			co[c].o=vis_loadobject(tmpname);
			memset(co[c].o->r,0,sizeof(rmatrix));
			memset(co[c].o->r0,0,sizeof(rmatrix));
			co[c].index=e;
			co[c].on=0;
			if(!indemo) printf("(co[%i]:%s)\n",c,co[c].o->name);
		}
		else
		{
			if(!indemo) printf("Copying %s.%03i... ",scene,e);
			for(g=0;g<c;g++) if(co[g].index==e) break;
			memcpy(co+c,co+g,sizeof(struct s_co));
			co[c].o=getmem(sizeof(object));
			memcpy(co[c].o,co[g].o,sizeof(object));
			co[c].o->r=getmem(sizeof(rmatrix));
			co[c].o->r0=getmem(sizeof(rmatrix));
			memset(co[c].o->r,0,sizeof(rmatrix));
			memset(co[c].o->r0,0,sizeof(rmatrix));
			co[c].on=0;
			if(!indemo) printf("(co[%i]:%s)\n",c,co[c].o->name);
		}
	}
	co[0].o=&camobject;
	camobject.r=&cam;
	camobject.r0=&cam;

	sprintf(tmpname,"%s.0AA",scene);
	if(!indemo) printf("Loading animations... %s\n",tmpname);
	ip=(short *)readfile(tmpname);
	while(*ip)
	{
		a=*ip;
		if(a==-1) break;
		sprintf(tmpname,"%s.0%c%c",scene,a/10+'A',a%10+'A');
		if(!indemo) printf("Scene: %s ",tmpname);
		scenelist[scl].data=readfile(tmpname);
		printf("(%i:@%p)\n",scl,scenelist[scl].data);
		scl++;
		ip+=2;
	}

	if(!indemo) 
	{
		printf("Press any key to continue...\n");
		//getch();
	}	

	resetscene();

#if 0
	for(;;)
	{
		// DIS: muscode
		_asm
		{
			mov	bx,6
			int	0fch
			mov	a,cx	// ord
			mov	b,bx	// row
		}
		if(a>10 && b>46) break;
		if(dis_exit()) return 1;
	}
#endif

	if (init_graphics("U2A", argc, argv) < 0) {
		fprintf(stderr, "Can't init graphics\n");
		return -1;
	};

	init_opengl();
	set_fps(36);

 	vid_init(3); ////// oversample x 4
	cp=(char *)(scenem+16);
	vid_setpal(cp);

#if 0
	outp(0x3c8,0);
	for(a=0;a<768;a++) outp(0x3c9,cp[a]);
#endif
	vid_window(0L,319L,25L,174L,512L,9999999L);
	
	dis_setcopper(2,copper2);
	dis_partstart();
	xit=0;
	coppercnt=0;
	syncframe=0;
	avgrepeat=1;
	cl[0].ready=0;
	cl[1].ready=0;
	cl[2].ready=0;
	cl[3].ready=1;
	while(!dis_exit() && !xit)
	{
		int fov;
		int onum;
		long pflag;
		long dis;
		long l;
		object *o;
		rmatrix *r;

		// DIS: muscode
#if 0
		_asm
		{
			mov	bx,6
			int	0fch
			mov	a,cx	// ord
			mov	b,bx	// row
		}
#endif
	        if(a>11 && b>54) break;
		
		deadlock=0;
#if 0
		while(cl[clw].ready)
		{
			if(deadlock>16) break;
		}
#endif

		// Draw to free frame
		//vid_setswitch(clw,-1);
		vid_clearbg(bg);
		// Field of vision
		vid_cameraangle(fov);
		// Calc matrices and add to order list (only enabled objects)
		ordernum=0;
		/* start at 1 to skip camera */
		for(a=1;a<conum;a++) if(co[a].on)
		{
			order[ordernum++]=a;
			o=co[a].o;
			memcpy(o->r,o->r0,sizeof(rmatrix));
			calc_applyrmatrix(o->r,&cam);
			b=o->pl[0][1]; // center vertex
			co[a].dist=calc_singlez(b,o->v0,o->r);
		}
		// Zsort
		if(city==1)
		{
			co[2].dist=1000000000L; // for CITY scene, test
			co[7].dist=1000000000L; // for CITY scene, test
			co[13].dist=1000000000L; // for CITY scene, test
		}
		if(city==2)
		{
			co[14].dist=1000000000L; // for CITY scene, test
		}
		for(a=0;a<ordernum;a++) 
		{
			dis=co[c=order[a]].dist;
			for(b=a-1;b>=0 && dis>co[order[b]].dist;b--)
				order[b+1]=order[b];
			order[b+1]=c;
		}
		// Draw
		for(a=0;a<ordernum;a++)
		{
			//int	x,y;
			o=co[order[a]].o;
			vis_drawobject(o);
		}
		// **** Drawing completed **** //
		// calculate how many frames late of schedule
		//avgrepeat=(avgrepeat+(syncframe-currframe)+1)/2;
		//repeat=avgrepeat;
		swap_buffers();
		poll_event();

		repeat=adjust_framerate();
		if(repeat<1) repeat=1;
		cl[clw].frames=repeat;
		cl[clw].ready=1;
		clw++; clw&=3;
		// advance that many frames
		repeat=repeat;
		currframe+=repeat;
	    while(repeat-- && !xit)
	    {
		// parse animation stream for 1 frame
		onum=0;
		while(!xit)
		{
			a=*sp++;
			if(a==0xff)
			{
				a=*sp++;
				if(a<=0x7f)
				{
					fov=a<<8;
					break;
				}
				else if(a==0xff) 
				{
					resetscene();
					xit=1;
					continue;
				}
			}
			if((a&0xc0)==0xc0)
			{
				onum=((a&0x3f)<<4);
				a=*sp++;
			}
			onum=(onum&0xff0)|(a&0xf);
			b=0;
			switch(a&0xc0)
			{
			case 0x80 : b=1; co[onum].on=1; break;
			case 0x40 : b=1; co[onum].on=0; break;
			}
			
			#ifdef DEBUG
			if(b) fprintf(fr,"[%i (%s) ",onum,co[onum].on?"on":"off");
			else fprintf(fr,"[%i (--) ",onum);
			#endif
			if(onum>=conum)
			{
				return(3);
			}
			
			r=co[onum].o->r0;
			
			pflag=0;
			switch(a&0x30)
			{
			case 0x00 : break;
			case 0x10 : pflag|=*sp++; break;
			case 0x20 : pflag|=sp[0]; 
				    pflag|=(long)sp[1]<<8; 
				    sp+=2; break;
			case 0x30 : pflag|=sp[0]; 
				    pflag|=(long)sp[1]<<8; 
				    pflag|=(long)sp[2]<<16; 
				    sp+=3; break;
			}
			
			#ifdef DEBUG
			fprintf(fr,"pfl:%06lX",pflag);
			#endif
			
			l=lsget(pflag);
			r->x+=l;
			l=lsget(pflag>>2);
			r->y+=l;
			l=lsget(pflag>>4);
			r->z+=l;
			
			#ifdef DEBUG
			fprintf(fr," XYZ:(%i,%i,%i)",r->x,r->y,r->z);
			#endif

			if(pflag&0x40)
			{ // word matrix
				for(b=0;b<9;b++) if(pflag&(0x80<<b))
				{
					r->m[b]+=lsget(2);
				}
			}
			else
			{ // byte matrix
				for(b=0;b<9;b++) if(pflag&(0x80<<b))
				{
					r->m[b]+=lsget(1);
				}
			}

			#ifdef DEBUG
			fprintf(fr,"]\n");
			#endif
		}
	    }
	}
	dis_setcopper(2,NULL);

	//vid_setswitch(0,-1);
	vid_clearbg(bg);
	//vid_setswitch(1,-1);
	vid_clearbg(bg);
	//vid_setswitch(2,-1);
	vid_clearbg(bg);
	//vid_setswitch(3,-1);
	vid_clearbg(bg);
	
	if(!dis_indemo())
	{
		vid_deinit();
	}

	#ifdef DEBUG
	fclose(fr);
	#endif
	return(0);
}

//////////////////////////////////////////////////////////////////////////////

void	*getmem(long size)
{
	void	*p;
	if(size>160000L)
	{
		printf("GETMEM: attempting to reserved >160K (%li byte block)\n",size);
		exit(3);
	}
	p=calloc(size/16L+1,16);
	if(!p)
	{
		printf("GETMEM: out of memory (%li byte block)\n",size);
		exit(3);
	}
	return(p);
}

void	freemem(void *p)
{
	free(p);
}

char	*readfile(char *name)
{
	FILE	*f1;
	long	size;
	char huge *p,*p0;
	f1=fopen(name,"rb");
	if(!f1)
	{
		printf("File '%s' not found.",name);
		exit(3);
	}
	fseek(f1,0L,SEEK_END);
	p0=p=getmem(size=ftell(f1));
	fseek(f1,0L,SEEK_SET);
#if 0
	if(size>128000)
	{
		fread(p,64000,1,f1);
		size-=64000; 
		_asm add word ptr p[2],4000
		fread(p,64000,1,f1);
		size-=64000; 
		_asm add word ptr p[2],4000
		fread(p,(size_t)size,1,f1);
	}
	else if(size>64000)
	{
		fread(p,64000,1,f1);
		size-=64000; 
		_asm
		{
			add word ptr p[2],4000
		}
		fread(p,(size_t)size,1,f1);
	}
	else
#endif
	fread(p,(size_t)size,1,f1);
	fclose(f1);
	return(p0);
}

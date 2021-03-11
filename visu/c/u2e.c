#include <stdio.h>
//#include <conio.h>
#include <string.h>
#include <stdlib.h>
//#include <malloc.h>
#include "graphics.h"
#include "../dis/dis.h"
#include "../opengl.h"
#include "../cd.h"
#include "../c.h"

#define	noDEBUG
#define huge
#define _loadds

int	indemo=0;

char	scene[64]={"U2E"};
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

char	fpal[768];

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

void border(int r,int g,int b)
{
#if 0
	outp(0x3c8,255);
	outp(0x3c9,r);
	outp(0x3c9,g);
	outp(0x3c9,b);
#endif
}

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
	int	ready;  // 1=ready for display
		 	// 2=displayed, still on screen
			// 0=free (not on screen)
} cl[4];
int	clr=0,clw=0;
int	firstframe=1;
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

void	fadeset(char *vram)
{
#if 0
	int	y;
	outp(0x3c4,2);
	outp(0x3c5,15);
	for(y=0;y<25;y++)
	{
		memset(vram+y*80+0,0,17);
		memset(vram+y*80+17,252,47);
		outp(0x3c4,2);
		outp(0x3c5,2+4+8);
		*(vram+y*80+63)=0;
		outp(0x3c4,2);
		outp(0x3c5,15);
		memset(vram+y*80+252,0,16);
	}
	for(y=25;y<175;y++)
	{
		memset(vram+y*80+0,254,17);
		memset(vram+y*80+17,253,47);
		outp(0x3c4,2);
		outp(0x3c5,2+4+8);
		*(vram+y*80+63)=254;
		outp(0x3c4,2);
		outp(0x3c5,15);
		memset(vram+y*80+64,254,16);
	}
	for(y<175;y<200;y++)
	{
		memset(vram+y*80+0,0,17);
		memset(vram+y*80+17,252,47);
		outp(0x3c4,2);
		outp(0x3c5,2+4+8);
		*(vram+y*80+63)=0;
		outp(0x3c4,2);
		outp(0x3c5,15);
		memset(vram+y*80+64,0,16);
	}
#endif
}

int main(int argc,char *argv[])
{
	//char huge *sptemp;
	short	huge *ip;
	//unsigned int u;
	char	huge *cp;
	int	jellywas=0;
	int	a,b,c,d,e,f,g/*,x,y,z*/;
	#ifdef DEBUG
	//fr=fopen("tmp","wt");
	fr=stdout;
	#endif
	indemo=0;
	
#if 0
	_asm
	{
		mov	ah,0fh
		int	10h
		xor	ah,ah
		mov	a,ax
	}
	if(a>3) jellywas=1;
#endif

	dis_partstart();
	sprintf(tmpname,"%s.00M",scene);
	if(!indemo) printf("Loading materials %s...\n",tmpname);
	scene0=scenem=readfile(tmpname);

	if(scene0[15]=='C') city=1;
	if(scene0[15]=='R') city=2;
	ip=(short *)(scene0+LONGAT(scene0+4));
	conum=d=*ip++;
	for(f=-1,c=1;c<d;c++)
	{	
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
		if(!indemo) printf("(%i:@%p)\n",scl,scenelist[scl].data);
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
    	if(!jellywas) 
	{
		vid_init(1);
		for(a=0;a<768;a++) fpal[a]=0;
		for(a=3;a<64*3;a++) fpal[a]=63;
	}
	else
	{
		outp(0x3c7,0);
		for(a=0;a<768;a++) fpal[a]=inp(0x3c9);
	}
	
	for(b=0;b<33;b++)
	{
		for(a=3;a<768-6;a++) 
		{
			fpal[a]+=2;
			if(fpal[a]>63) fpal[a]=63;
		}
		dis_waitb();
		outp(0x3c8,0);
		for(a=0;a<768;a++) outp(0x3c9,fpal[a]);
	}
#endif

	if (!jellywas) {
		if (init_graphics("U2E", argc, argv) < 0) {
			fprintf(stderr, "Can't init graphics\n");
			return -1;
		};

		init_opengl();
		set_fps(36);

		vid_init(1);
	}

	// tall rectangle
	// start white, we can't reuse image from previous part
	// (jelly has to fade to white rectangle at end)

#if 0
	for(b=0;b<16;b++)
	{
		dis_waitb();
	}
	
	{
		fadeset((char *)0xa0000000L);
		dis_waitb();
		outp(0x3d4,9);
		a=inp(0x3d5);
		a=(a&0xf0)|0x80;
		outp(0x3d5,a);
		dis_waitb();
		fadeset((char *)0xa4000000L);
		fadeset((char *)0xa8000000L);
		fadeset((char *)0xac000000L);
		//swap_buffers();
	}
#endif

#define RMAX 0.93f

	float r1a = RMAX;
	float r2a = 0.0f;

	setrgb(1, 63, 63, 63, r1a);
	setrgb(2, 63, 63, 63, r2a);

	for(b=0;b<32 + 16;b++)
	{
		int repeat = adjust_framerate();
		if (repeat == 0) b--;
		else while (--repeat > 0) b++;

		clear_screen();
		draw_rectangle1();
		swap_buffers();
		poll_event();
	}

	// cross-fade to wide rectangle
	
	for(b=0;b<33;b++)
	{
		int repeat = adjust_framerate();
		if (repeat == 0) b--;
		else while (--repeat > 0) b++;

#if 0
		for(a=3;a<768-9;a++) 
		{
			fpal[a]-=2;
			if(fpal[a]<0) fpal[a]=0;
		}
		for(a=768-9;a<768-3;a++) 
		{
			fpal[a]+=2;
			if(fpal[a]>63) fpal[a]=63;
		}
		dis_waitb();

		outp(0x3c8,0);
		for(a=0;a<768;a++) outp(0x3c9,fpal[a]);
#endif
		
		clear_screen();

		draw_rectangle1();
		draw_rectangle2();
		
		r1a = RMAX * (32 - b) / 32;
		r2a = RMAX * b / 32;

		setrgb(1, 63, 63, 63, r1a);
		setrgb(2, 63, 63, 63, r2a);
		swap_buffers();
		poll_event();
	}

	// Wait a little bit -- not needed when music in place
	for(b=0;b<32 + 16;b++)
	{
		int repeat = adjust_framerate();
		if (repeat == 0) b--;
		else while (--repeat > 0) b++;

		clear_screen();
		draw_rectangle2();
		swap_buffers();
		poll_event();
	}

	clear_screen();
	for(a=0;a<256;a++) setrgb(a,fpal[a*3],fpal[a*3+1],fpal[a*3+2], 1.0f);

	//vid_init(11);

	cp=(char *)(scenem+16);
	cp[255*3+0]=0;
	cp[255*3+1]=0;
	cp[255*3+2]=0;
	cp[252*3+0]=0;
	cp[252*3+1]=0;
	cp[252*3+2]=0;
	cp[253*3+0]=63;
	cp[253*3+1]=63;
	cp[253*3+2]=63;
	cp[254*3+0]=63;
	cp[254*3+1]=63;
	cp[254*3+2]=63;
	vid_setpal(cp);
	vid_window(0,319,25,174,512,9999999);
	
	dis_setcopper(2,copper2);
	dis_partstart();
	xit=0;

#if 0
	//DIS: muscode
	while(!dis_exit())
	{
		_asm
		{
			mov	bx,6
			int	0fch
			mov	a,cx
		}
		if(a>18) break;
	}
#endif
	
	coppercnt=0;
	syncframe=0;
	avgrepeat=1;
	cl[0].ready=0;
	cl[1].ready=0;
	cl[2].ready=0;
	cl[3].ready=1;
	
	int first = 2;

	while(!dis_exit() && !xit)
	{
		int fov;
		int onum;
		long pflag;
		long dis;
		long l;
		object *o;
		rmatrix *r;

	    if(!firstframe)
	    {
 		deadlock=0;
#if 0
		while(cl[clw].ready)
		{
			if(deadlock>16) break;
		}
#endif
		// Draw to free frame
		//vid_setswitch(clw,-1);
		vid_clear255();
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
			if(co[a].o->name[1]=='_') co[a].dist=1000000000L;
			else co[a].dist=calc_singlez(b,o->v0,o->r);
			if(currframe>900*2 && currframe<1100*2)
			{
				if(co[a].o->name[1]=='s' &&
				   co[a].o->name[2]=='0' &&
				   co[a].o->name[3]=='1')
				   	co[a].dist=1L;
			}
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
	    } else { syncframe=0; firstframe=0; coppercnt=1; }
		// calculate how many frames late of schedule
		//a=(syncframe-currframe);
		//repeat=a+1;
		if (!first)
		{
			swap_buffers();
			poll_event();
		}
		if (first > 0) first--;

		repeat=adjust_framerate();
		if(repeat<0) repeat=0;
		/*if(repeat==0) cl[clw].frames=1;
		else*/ cl[clw].frames=repeat;
		cl[clw].ready=1;
		clw++; clw&=3;
		// advance that many frames
		//repeat=(repeat+1)/2;
		currframe+=repeat*2;
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

#if 0
	outp(0x3c7,0);
	for(a=0;a<768;a++) fpal[a]=inp(0x3c9);
#endif
	//for(a=0;a<256;a++) getrgb(a,&fpal[a*3]);


	for(b=0;b<16;b++)
	{
#if 0
		for(a=0;a<768;a++) 
		{
			fpal[a]+=4;
			if(fpal[a]>63) fpal[a]=63;
		}
		dis_waitb();
		outp(0x3c8,255);
		for(a=0;a<768;a++) outp(0x3c9,fpal[a]);
#endif
		int repeat = adjust_framerate();
		if (repeat == 0) b--;
		else while (--repeat > 0) b++;

		for(a=0;a<ordernum;a++)
			vis_drawobject(co[order[a]].o);

		setrgb(255, 63, 63, 63, 1.0 * b / 15);

		draw_rectangle3();
		swap_buffers();
		poll_event();
	}
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

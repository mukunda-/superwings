#ifndef juju
#define juju

typedef struct  tboss_prototype
{
	struct tboss_prototype* next;
	u8 		flags;
	u8 		type;
	u8 		blaster1;
	u8 		stage;
	
	collbox cb_m;
	collbox cb_l;
	collbox cb_r;
	
	int 	x;
	int		y;
	u8		flash_l;
	u8		flash_m;
	u8		flash_r;
	u8		blaster2;
	
	u16		hp_left;
	u16		hp_middle;
	
	u16		hp_right;
	s16		scale_jitter;
	
	int		scale;
	
	u8		sdir;
	u8		slen;
	s16		zdir;
	
	u8		blaster3;
	u8		blash1;
	u8		blash2;
	u8		blash3;
} boss_prototype;

void prototypeSpawn();
void prototypeUnload( boss_prototype* p );
void prototypeUpdate( boss_prototype* p );
void prototypeDraw( boss_prototype* p );

#endif

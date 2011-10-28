void zoom_out ();

void zoom_in ();

void zoom_fahrt(int value);

float distanzx(float x);

float distanzy(float x);

float distanzz(float x);

float positionx(void);

float positiony(void);

float positionz(void);

int findangle();

int findangle_x();

void delay(int value);			

void anim(int value);			

void animation(float zu_x,float zu_y,float zu_z);	// kameraflug von der momentanen position

void rotate_y_li(float);

void rotate_y_re(float);

void rotate_x_up(float);			//drehung um x-achse nach oben

void rotate_x_down(float);			//drehung um x-achse nach unten

void kamerafahrt(int value);

void animation_twopoints(float von_x,float von_y,float von_z, float zu_x,float zu_y,float zu_z);

void kamerafahrt_endl(int value);

void kamerafahrt_zuplayer(int value);

float findangle_360();

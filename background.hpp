#ifndef BACKGROUND_HPP_HEADERFILE
#define BACKGROUND_HPP_HEADERFILE

void initialise_bg();
void update_bground(uint32_t hh, uint32_t mm);
void draw_background(void);

extern int currentLocation;
int getNumLocations(void);
const char *getLocationName(int idx);

#endif
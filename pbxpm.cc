/*
 * Yury P. Fedorchenko 2010
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include "pbres.h"
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>

struct mxpm{
  int w,h,nc;
  bool read,pal_read,finish;
  std::map<char,std::string> pal;
  std::map<char,unsigned char> pal_pb;
  std::vector<std::string> lines;
  mxpm():read(false),pal_read(false),finish(false){}
};


unsigned char pal2pb(const std::string& ss){
    std::string s;
    s+='0';s+=ss;
    int val;
    sscanf(s.c_str(),"%x",&val);
    unsigned char* pos=reinterpret_cast<unsigned char* >(&val);
    
    unsigned char c = (pos[0] + pos[1]*6 + pos[2]*3) / 10;
    if (c <= 0x2a) {
	return 0;
    } else if (c <= 0x80) {
	return 1;
   } else if (c <= 0xd4) {
	return 2;
   } else {
	return 3;
   }
}

std::ostream& operator<<(std::ostream& s,const mxpm& p){
  for(std::map<char,std::string>::const_iterator it=p.pal.begin();it!=p.pal.end();++it){
    s<<it->first<<" c "<<it->second<<std::endl;
  }
  for(std::map<char,unsigned char>::const_iterator it=p.pal_pb.begin();it!=p.pal_pb.end();++it){
    s<<it->first<<" c "<<(int)it->second<<std::endl;
  }
  for(std::vector<std::string>::const_iterator it=p.lines.begin();it!=p.lines.end();++it){
    s<<*it<<std::endl;
  }
  return s;
}

ibitmap *xpm2res(const char *fname, unsigned char* data,int len,int *outsize) {
	std::cerr<<"Processing:"<<fname<<std::endl;
	ibitmap *bm;
	unsigned char *udata, *umask, *outp, *maskp;
	int i, x, y, usemask;

#ifndef CONVERT
	std::stringstream in((const char*)data);
#else
	std::ifstream in(fname);
#endif
	mxpm px;
	while(!in.eof() && !in.fail()){
	  std::string str;
	  std::getline(in,str);
	  size_t ff=str.find_first_of("\"");
	  if(ff!=str.npos){
	    if(!px.read){
	      std::stringstream ss(str.substr(ff+1));
	      ss>>px.w>>px.h>>px.nc;
	      if(px.nc==0)terminate("error in xpm file %s %s\n",fname,str.c_str());
	      else px.read=true;
	    }else if(!px.pal_read){
	      std::string ss(str.substr(ff+1));
	      char cn,empt;
	      std::string rest;
	      cn=ss[0];
//	      ss>>cn>>empt>>rest;
	      rest=ss.substr(ss.find_last_of("#N"));
	      px.pal[cn]=rest.substr(0,rest.find_last_of("\""));
	      if(px.pal[cn]=="None" || px.pal[cn]=="Null")px.pal_pb[cn]=255;
	      else{rest=px.pal[cn];rest[0]='x';
	      px.pal_pb[cn]=pal2pb(rest);}
	      if(px.pal.size()==px.nc)px.pal_read=true;
	    }else{
	      px.lines.push_back(str.substr(ff+1,str.find_last_of("\"")));
	      if(px.lines.size()==px.h){
	      px.finish=true;
	      break;
	      }
	    }
	  }
	}
	
	int scanout = (px.w + 3) / 4;
	int maskout = (px.w + 7) / 8;


	int ulen = px.h * scanout;
	int umasklen = px.h * maskout;

	bm = (ibitmap*)calloc(sizeof(ibitmap) + ulen + umasklen +10, 1);
	bm->width = px.w;
	bm->height = px.h;
	bm->depth = 2;//(bpp == 1) ? 1 : 2;
	bm->scanline = scanout;

	udata = bm->data;
	memset(udata, 0, ulen);
	umask = bm->data + ulen;
	memset(umask, 0xff, umasklen);
	usemask = 0;

	for (y=0; y<px.h; y++) {

		outp = udata + (y) * scanout;
		maskp = umask + (y) * maskout;

		/*if (bpp == 1) {

			inv = (palette[0] < palette[1]) ? 0xff : 0;
			for (i=0; i<scanout; i++) {
				outp[i] = inp[i] ^ inv;
			}

		} else {*/

			for (x=0; x<px.w; x++) {
				if (px.pal_pb[px.lines[y][x]] != 255) {
					outp[x>>2] |= (px.pal_pb[px.lines[y][x]]) << ((3 - (x & 3)) << 1);
				} else {
					usemask = 1;
					maskp[x>>3] &= ~(1 << (7 - (x & 7)));
				}

			}

		//}

	}

	if (usemask) bm->depth |= 0x8000;
	*outsize = usemask ? ulen+umasklen : ulen;
	std::cerr<<"end Processing:"<<fname<<std::endl;
	return bm;

}

#ifdef CONVERT
void write_as_c(FILE *f, unsigned char *data, int len) {

	char buf[512];
	int i, n, ll;

	while (len > 0) {

		n = (len > 16) ? 16 : len;

		ll = 0;
		ll += sprintf(buf+ll, "\t\t");
		for (i=0; i<n; i++) ll += sprintf(buf+ll, "0x%02x,", data[i]);
		ll += sprintf(buf+ll, "\n");

		if (fwrite(buf, 1, ll, f) != ll) terminate("error writing to file");

		data += n;
		len -= n;

	}

}
void terminate(const char *fmt, ...) {

	va_list ap;

	fprintf(stderr, "Error: ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n\n");
	exit(1);

}
char *getname(char *s) {

	static char tname[64];
	char *p;

	p = strrchr(s, '/');
	if (p == NULL) p = strrchr(s, '\\');
	if (p != NULL) p++;
	if (p == NULL) p = s;
	strncpy(tname, p, 63);
	p = strchr(tname, '.');
	if (p != NULL) *p = 0;
	return tname;

}

void usage(const char* nm){
  std::cerr<<"usage"<<std::endl;
  exit(1);
}

int main(int argc,char** argv){
  int beg;
  FILE* out=stdout;
  if(argc<2)usage(argv[0]);
  if(strcmp(argv[1],"-o")==0){
    if(argc>4)
    out=fopen(argv[2],"w");
    else
    usage(argv[0]);
  }
  for(int i=1;i<argc;++i){
    int size;
    ibitmap *bm=xpm2res(argv[i],NULL,0,&size);
    fprintf(out, "const ibitmap %s = {\n", getname(argv[i]));
		fprintf(out, "\t%u, %u, %u, %u,\n", bm->width, bm->height, bm->depth, bm->scanline);
		fprintf(out, "\t{\n");
		write_as_c(out, bm->data, size);
		fprintf(out, "\t}\n");
		fprintf(out, "};\n\n");

  }
  return 0;
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <locale.h>

#include "../LibAAF/libAAF.h"
/*
#include "AAFClass.h"
#include "AAFUtils.h"
#include "AAFIface.h"

#include "AAFDefs/AAFClassDefUIDs.h"
#include "AAFDefs/AAFTypeDefUIDs.h"
#include "AAFDefs/AAFPropertyIDs.h"
#include "AAFDefs/AAFDataDefs.h"

#include "AAFToText.h"
#include "AAFCore.h"
#include "AAFNavigation.h"
*/

#include "./Ardour/Ardour.h"
/*
#ifndef auidCmp
#define auidCmp( auid1, auid2 ) \
	( *(uint64_t*)auid1     == *(uint64_t*)auid2 || \
	  *(uint64_t*)(auid1+8) == *(uint64_t*)(auid2+8) )
#endif
*/



/*
void printObjectProperties( aafObject *Obj )
{
	aafProperty * Prop = NULL;

	for ( Prop = Obj->properties;  Prop != NULL; Prop = Prop->next )
	{
		printf( ":.: %s\n", PIDToText( Prop->pid ) );
	}
}
*/





/*
#define foreachAudioEssence( ae, aeList ) \
	for ( ae = aeList; ae != NULL; ae = ae->next )


#define foreachAudioClip( ac, acList ) \
	for ( ac = acList; ac != NULL; ac = ac->next )
*/


int main( int argc, char *argv[] )
{
	if ( argc < 2 )
		return 1;

	// AAF_Data *aafd = aaf_alloc();


//	printClasses( aafd.Class, 0 );


//	AAF_Iface *aafi = calloc( 1, sizeof(AAF_Iface) );

	AAF_Iface *aafi = aafi_alloc( NULL );

	if ( aaf_load_file( aafi->aafd, argv[argc-1] ) )
		return 1;

	// aafi->aafd = aafd;

	retrieveEssences( aafi );
	retrieveClips( aafi );

	aafiAudioEssence *ae = NULL;


	char progPath[1024];
	memset(progPath, 0x00, 1024);

	readlink( "/proc/self/exe", progPath, 1024 );

	progPath[strlen(progPath)-10] = 0x00;



	uint32_t i = 0;

	foreachAudioEssence( ae, aafi->Audio->Essences )
	{
		char file[255];

		snprintf( file, 255, "%sArdour/session/interchange/testF3/audiofiles/aaf_audio_%d", progPath, i++ );

		extractAudioEssence( aafi, ae, file );
	}


//	aafiAudioClip *ac = NULL;



	size_t buf_sz = 400000;

	char *buf = malloc( buf_sz );
	memset( buf, 0x00, buf_sz );

	size_t offset = 0;





	offset += snprintf( buf+offset, buf_sz-offset, "  <Sources>\n" );


	aafiAudioTrack   *audioTrack = NULL;
	aafiTimelineItem *audioItem  = NULL;
	aafiAudioClip    *audioClip  = NULL;
	aafiAudioEssence *audioMedia = NULL;

	foreachAudioEssence( audioMedia, aafi->Audio->Essences )
	{
		offset += snprintf( buf+offset, buf_sz-offset, "    <Source name=\"%s.wav\" type=\"audio\" flags=\"\" id=\"%u\" captured-for=\"PAD 1\" channel=\"0\" origin=\"\" gain=\"1\"/>\n",
						audioMedia->file,
						(uint16_t)((uint64_t)((&audioMedia->sourceMobID)) & 0xffff) );

	}

	offset += snprintf( buf+offset, buf_sz-offset, "  </Sources>\n" );




	offset += snprintf( buf+offset, buf_sz-offset, "  <Regions>\n" );

	foreach_audioTrack( audioTrack, aafi )
	{

		foreach_audioItem( audioItem, audioTrack )
		{

			if ( audioItem->type != AAFI_CLIP )
				continue;

			audioClip = (aafiAudioClip*)&audioItem->data;

			offset += snprintf( buf+offset, buf_sz-offset, "<Region name=\"%s\" muted=\"0\" opaque=\"1\" locked=\"0\" video-locked=\"0\" automatic=\"1\" whole-file=\"1\" import=\"0\" external=\"0\" sync-marked=\"0\" left-of-split=\"0\" right-of-split=\"0\" hidden=\"0\" position-locked=\"0\" valid-transients=\"0\" start=\"%li\" length=\"%li\" position=\"%li\" beat=\"0\" sync-position=\"0\" ancestral-start=\"0\" ancestral-length=\"0\" stretch=\"1\" shift=\"1\" positional-lock-style=\"AudioTime\" layering-index=\"0\" envelope-active=\"0\" default-fade-in=\"0\" default-fade-out=\"0\" fade-in-active=\"1\" fade-out-active=\"1\" scale-amplitude=\"1\" id=\"%u\" type=\"audio\" first-edit=\"nothing\" source-0=\"%u\" master-source-0=\"%u\" channels=\"%u\"/>\n",
	 					audioClip->Essence->file,
/*						ac->essenceStartOffset * (48000/25),*/
						eu2sample( audioClip, audioClip->essence_offset ),
/*						ac->length * (48000/25),*/
						eu2sample( audioClip, audioClip->len ),
/*						ac->timelinePos * (48000/25),*/
						eu2sample( audioClip, (audioClip->pos + audioClip->track->Audio->tc->start) ),
						(uint16_t)((uint64_t)(&audioClip->Essence->sourceMobID) & 0xffff ) + 1,
						(uint16_t)((uint64_t)(&audioClip->Essence->sourceMobID) & 0xffff ),
						(uint16_t)((uint64_t)(&audioClip->Essence->sourceMobID) & 0xffff ),
						audioClip->track->number );
		}
	}

	offset += snprintf( buf+offset, buf_sz-offset, "  </Regions>\n" );







	offset += snprintf( buf+offset, buf_sz-offset, "  <Playlists>\n" );

	foreach_audioTrack( audioTrack, aafi )
	{

		offset += snprintf( buf+offset, buf_sz-offset, "    <Playlist id=\"18772\" name=\"Audio %i.1\" type=\"audio\" orig-track-id=\"18730\" shared-with-ids=\"\" frozen=\"0\" combine-ops=\"0\">\n", audioTrack->number /*i*/ );

			foreach_audioItem( audioItem, audioTrack )
			{

				if ( audioItem->type != AAFI_CLIP )
					continue;

				audioClip = (aafiAudioClip*)&audioItem->data;

				char name[255];

				snprintf( name, 255, "%s.%u",
							audioClip->Essence->file,
							audioClip->subClipNum );

			offset += snprintf( buf+offset, buf_sz-offset, "      <Region name=\"%s\" muted=\"0\" opaque=\"1\" locked=\"0\" video-locked=\"0\" automatic=\"1\" whole-file=\"0\" import=\"0\" external=\"0\" sync-marked=\"0\" left-of-split=\"0\" right-of-split=\"0\" hidden=\"0\" position-locked=\"0\" valid-transients=\"0\" start=\"%li\" length=\"%li\" position=\"%li\" beat=\"0\" sync-position=\"0\" ancestral-start=\"0\" ancestral-length=\"0\" stretch=\"1\" shift=\"1\" positional-lock-style=\"AudioTime\" layering-index=\"0\" envelope-active=\"0\" default-fade-in=\"0\" default-fade-out=\"0\" fade-in-active=\"1\" fade-out-active=\"1\" scale-amplitude=\"1\" id=\"%u\" type=\"audio\" first-edit=\"nothing\" source-0=\"%u\" master-source-0=\"%u\" channels=\"1\"></Region>\n",
		 					name,
/*							ac->essenceStartOffset * (48000/25),*/
							eu2sample( audioClip, audioClip->essence_offset ),
/*							ac->length * (48000/25),*/
							eu2sample( audioClip, audioClip->len ),
/*							ac->timelinePos * (48000/25),*/
							eu2sample( audioClip, (audioClip->pos + audioClip->track->Audio->tc->start) ),
							(uint16_t)(((uint64_t)(&audioClip->Essence->sourceMobID)-audioClip->subClipNum) & 0xffff),
							(uint16_t)((uint64_t)(&audioClip->Essence->sourceMobID) & 0xffff),
							(uint16_t)((uint64_t)(&audioClip->Essence->sourceMobID) & 0xffff ) );
			}


		offset += snprintf( buf+offset, buf_sz-offset, "    </Playlist>\n" );
	}


	offset += snprintf( buf+offset, buf_sz-offset, "  </Playlists>\n" );

	writeArdourSessionFile( buf );

	free( buf );

	aaf_release( &(aafi->aafd) );

	return 0;
}

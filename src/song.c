#include"song.h"

// Read XML file and extract song data
void xml_parse_songfile(char*fn)
{
	xmlTextReaderPtr reader=xmlReaderForFile(fn, NULL, 0);

	if(reader)
	{
		int found_song=0,found_length=0,found_notes=0;

		while(xmlTextReaderRead(reader))
		{
			//printf("'%s' => '%s'\n",xmlTextReaderConstName(reader),xmlTextReaderConstValue(reader));
			const char*name=xmlTextReaderConstName(reader);

			if(strcmp(name,"length")==0&&(found_length=!found_length))
			{
				xmlTextReaderRead(reader);
				printf("length: %s\n",xmlTextReaderConstValue(reader));
			}

			if(strcmp(name,"notes")==0&&(found_notes=!found_notes))
			{
				xmlTextReaderRead(reader);
				printf("notes: '%s'\n",xmlTextReaderConstValue(reader));
			}
		}
		xmlFreeTextReader(reader);
	}
	else
		printf("Unable to open %s\n", fn);

	xmlCleanupParser();
	xmlMemoryDump();
}

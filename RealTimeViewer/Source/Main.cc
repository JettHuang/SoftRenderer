// \brief
//		main entry.
//

#include "App.h"

int main(int argc, char* argv[])
{
	FApp theApp;

	if (theApp.Initialize("RealTimeViewer", 1024u, 768u))
	{
		theApp.MainLoop();
		theApp.Uninitialize();
	}

	return 0;
}

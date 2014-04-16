#include "reportdesignermainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setApplicationName("reportdesigner");
	a.setApplicationVersion("0.0.1");
	a.setOrganizationDomain("freesoftware.co.ke");
	a.setOrganizationName("Free Software Foundation(K) Ltd");

	ReportDesignerMainWindow w;
	w.show();

	return a.exec();
}

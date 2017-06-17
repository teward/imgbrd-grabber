#include "test-suite.h"


TestSuite::TestSuite()
	: QObject(), profile("tests/resources/")
{
	getSuites().append(this);
}

void TestSuite::setupSource(QString site)
{
	QDir().mkpath("tests/resources/sites/" + site);
	QFile::remove("tests/resources/sites/" + site +"/model.xml");
	QFile::copy("release/sites/" + site +"/model.xml", "tests/resources/sites/" + site +"/model.xml");
}

void TestSuite::setupSite(QString site, QString source)
{
	QDir().mkpath("tests/resources/sites/" + site + "/" + source);
	QFile::remove("tests/resources/sites/" + site +"/" + source + "/settings.ini");
	if (QFile::exists("release/sites/" + site +"/" + source + "/settings.ini"))
	{ QFile::copy("release/sites/" + site +"/" + source + "/settings.ini", "tests/resources/sites/" + site +"/" + source + "/settings.ini"); }
}

QList<QObject*> &TestSuite::getSuites()
{
	static QList<QObject*> suites;
	return suites;
}

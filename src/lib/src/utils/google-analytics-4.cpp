#include "google-analytics-4.h"
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QVariant>
#include <QNetworkAccessManager>
#include <QOperatingSystemVersion>

#ifdef QT_GUI_LIB
	#include <QGuiApplication>
	#include <QScreen>
#endif
#ifdef Q_OS_ANDROID
	#include <QAndroidJniObject>
#endif

#define MEASUREMENT_ENDPOINT_JSON "https://www.google-analytics.com/mp/collect"
#define MEASUREMENT_ENDPOINT_WEB "https://www.google-analytics.com/g/collect"
#define CLIENT_ID_SETTINGS_KEY "QtGoogleAnalytics4/ClientId"


#include <QSettings>
#include "functions.h"
GoogleAnalytics4::GoogleAnalytics4(QObject *parent)
	: QObject(parent)
{
	m_networkAccessManager = new QNetworkAccessManager(this);
	m_sessionId = QDateTime::currentMSecsSinceEpoch() / 1000; // QRandomGenerator::global()->generate();

	QSettings settings(savePath("settings.ini"), QSettings::IniFormat);
	if (!settings.contains(CLIENT_ID_SETTINGS_KEY)) {
		m_clientId = QUuid::createUuid().toString().mid(1, 36);
		settings.setValue(CLIENT_ID_SETTINGS_KEY, m_clientId);
	} else {
		m_clientId = settings.value(CLIENT_ID_SETTINGS_KEY).toString();
	}
}


void GoogleAnalytics4::setApiSecret(const QString &apiSecret)
{
	if (m_apiSecret != apiSecret) {
		m_apiSecret = apiSecret;
		emit apiSecretChanged();
	}
}

QString GoogleAnalytics4::apiSecret() const
{
	return m_apiSecret;
}

void GoogleAnalytics4::setMeasurementId(const QString &measurementId)
{
	if (m_measurementId != measurementId) {
		m_measurementId = measurementId;
		emit measurementIdChanged();
	}
}

QString GoogleAnalytics4::measurementId() const
{
	return m_measurementId;
}

void GoogleAnalytics4::setUserId(const QString &userId)
{
	if (m_userId != userId) {
		m_userId = userId;
		emit userIdChanged();
	}
}

QString GoogleAnalytics4::userId() const
{
	return m_userId;
}

#include <QEventLoop>
#include <QNetworkReply>

void GoogleAnalytics4::sendEvent(const QString &name, const QVariantMap &parameters)
{
	QUrl url(MEASUREMENT_ENDPOINT_WEB);

	QUrlQuery query {
		{ "v", "2" },
		{ "tid", m_measurementId },
		{ "_p", QString::number(QRandomGenerator::global()->generate()) },
		{ "cid", m_clientId },
		{ "ul", QLocale::system().name().toLower().replace("_", "-") },
		#ifdef QT_GUI_LIB
			{ "sr", screenResolution() },
		#endif
		{ "_s", "1" },
		{ "sid", QString::number(m_sessionId) },
		{ "sct", "1" },
		{ "en", name },
	};
	if (!m_userId.isEmpty()) {
		query.addQueryItem("uid", m_userId);
	}

	// Operating System information
	const QOperatingSystemVersion os = QOperatingSystemVersion::current();
	query.addQueryItem("uaa", "x86"); // QSysInfo::currentCpuArchitecture());
	query.addQueryItem("uab", QString::number(sizeof(void*) * 8));
	query.addQueryItem("uamb", os.isAnyOfType({ QOperatingSystemVersion::Android, QOperatingSystemVersion::IOS }) ? "1" : "0");
	#if defined(Q_OS_ANDROID)
		query.addQueryItem("uam", QAndroidJniObject::getStaticObjectField<jstring>("android/os/Build", "MODEL").toString());
	#elif defined(Q_OS_IOS)
		query.addQueryItem("uam", "iPhone");
	#else
		query.addQueryItem("uam", "");
	#endif
	query.addQueryItem("uap", os.name());
	query.addQueryItem("uapv", userAgentPlatformVersion());
	query.addQueryItem("uaw", "0");

	// Events
	for (auto it = parameters.constBegin(); it != parameters.constEnd(); ++it) {
		if (it.value().type() == QVariant::Type::Int) {
			query.addQueryItem("epn." + it.key(), QString::number(it.value().toInt()));
		} else {
			query.addQueryItem("ep." + it.key(), it.value().toString());
		}
	}

	// TODO: user_properties

	url.setQuery(query);

	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::UserAgentHeader, userAgent());
	QNetworkReply *reply = m_networkAccessManager->get(request);

	connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}


#ifdef QT_GUI_LIB
	QString GoogleAnalytics4::screenResolution() const
	{
		const QScreen *screen = QGuiApplication::primaryScreen();
		if (screen == nullptr) {
			return {};
		}

		const QSize size = screen->size();
		return QString("%1x%2").arg(size.width()).arg(size.height());
	}
#endif

QString GoogleAnalytics4::userAgent() const
{
	#if false and defined(Q_OS_ANDROID)
		QAndroidJniObject jsText = QAndroidJniObject::fromString("http.agent");
		QAndroidJniObject ua = QAndroidJniObject::callStaticMethod<jstring>(
		"System",
		"getProperty",
		"(Ljava/lang/String;)Z",
		jsText.object<jstring>()
		);
		return ua.toString();
	#endif

	return "";

	const QString appName = QCoreApplication::instance()->applicationName();
	const QString appVersion = QCoreApplication::instance()->applicationVersion();
	const QString systemInfo = QString("%1 %2").arg(QOperatingSystemVersion::current().name(), userAgentPlatformVersion());

	return QString("%1/%2 (%3) QtGoogleAnalytics4/1.0 (Qt/%4)").arg(appName, appVersion, systemInfo, QT_VERSION_STR);
}

QString GoogleAnalytics4::userAgentPlatformVersion() const
{
	const QOperatingSystemVersion os = QOperatingSystemVersion::current();

	QString ret;
	const int segmentCount = os.segmentCount();
	if (segmentCount > 0) {
		ret += QString("%1").arg(os.majorVersion());
	}
	if (segmentCount > 1) {
		ret += QString(".%1").arg(os.minorVersion());
	}
	if (segmentCount > 2) {
		ret += QString(".%1").arg(os.microVersion());
	}
	return ret;
}

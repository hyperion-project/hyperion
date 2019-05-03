
// Local-Hyperion includes
#include "LedDeviceAurora.h"
#include <netdb.h>
#include <assert.h>
// jsoncpp includes
#include <json/json.h>
// qt includes
#include <QEventLoop>
#include <QNetworkReply>

#define ROOT_ROUTE ""
#define EFFECTS_ROUTE "effects"
#define URL_FORMAT "http://%1:16021/api/v1/%2/%3"
#define EXT_MODE_STRING "{\"write\" : {\"command\" : \"display\", \"animType\" : \"extControl\"}}"
#define PANELLAYOUT_KEY "panelLayout"
#define PANELCOUNT_KEY "numPanels"
#define PANELID_KEY "panelId"
#define POSITIONDATA_KEY "positionData"
#define UDPPORT_KEY "streamControlPort"
#define RHYTHM_KEY "rhythm"
#define RHYTHM_CONNECTED_KEY "rhythmConnected"

struct addrinfo vints, *serverinfo, *pt;
//char udpbuffer[1024];
int sockfp;
int update_num;

LedDeviceAurora::LedDeviceAurora(const std::string& output, const std::string&  key) {
	manager = new QNetworkAccessManager();
	std::string hostname = output;
	std::string port;
	Json::Reader reader;
	Json::FastWriter writer;
	// Read Panel count and panel Ids
	QByteArray response = get(QString::fromStdString(hostname), QString::fromStdString(key), ROOT_ROUTE);
	Json::Value json;
	if (!reader.parse(QString(response).toStdString(), json)) {
		throw std::runtime_error("No Layout found");
	}
	//Debug
	// Json::FastWriter fastWriter;
	// std::cout << fastWriter.write(json);

	Json::Value rhythmJson = json[RHYTHM_KEY];
	json = json[PANELLAYOUT_KEY];
	panelCount = json[PANELCOUNT_KEY].asUInt();
	Json::Value positionDataJson = json[POSITIONDATA_KEY];
	// Loop over all children.
	for (Json::ValueIterator it = positionDataJson.begin(); it != positionDataJson.end() && panelIds.size() < panelCount; it++) {
		Json::Value panel = *it;		
		int panelId = panel[PANELID_KEY].asUInt();
		panelIds.push_back(panelId);
	}

	// Check if we found enough lights.
	if ((panelIds.size() != panelCount) || (panelIds.size() != panelCount-1 && rhythmJson[RHYTHM_CONNECTED_KEY].asBool())) {
		throw std::runtime_error("Not enough pannels found");
	}else {
		std::cout <<  "All panel Ids found: "<< panelIds.size()<<std::endl;
	}
	// Set Aurora to UDP Mode
	QByteArray modeResponse = changeMode(QString::fromStdString(hostname), QString::fromStdString(key), EFFECTS_ROUTE);
	Json::Value configJson;
	if (!reader.parse(QString(modeResponse).toStdString(), configJson)) {
		throw std::runtime_error("Could not change Mode token ");
	}
	//std::cout << fastWriter.write(configJson);
	// Get UDP port
	port = std::to_string(configJson[UDPPORT_KEY].asUInt());

	//std::cout << "output " << output << " hostname " << hostname << " port " << port <<std::endl;

	int rv;

	memset(&vints, 0, sizeof vints);
	vints.ai_family = AF_UNSPEC;
	vints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(hostname.c_str() , port.c_str(), &vints, &serverinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		assert(rv==0);
	}

	// loop through all the results and make a socket
	for(pt = serverinfo; pt != NULL; pt = pt->ai_next) {
		if ((sockfp = socket(pt->ai_family, pt->ai_socktype,
				pt->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (pt == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		assert(pt!=NULL);
	}
	//std::cout << "Started successfully ";
}

QString LedDeviceAurora::getUrl(QString host, QString token, QString route) {
	return QString(URL_FORMAT).arg(host).arg(token).arg(route);
}

QByteArray LedDeviceAurora::get(QString host, QString token, QString route) {
	QString url = getUrl(host, token, route);
	// Perfrom request
	QNetworkRequest request(url);
	QNetworkReply* reply = manager->get(request);
	// Connect requestFinished signal to quit slot of the loop.
	QEventLoop loop;
	loop.connect(reply, SIGNAL(finished()), SLOT(quit()));
	// Go into the loop until the request is finished.
	loop.exec();
	// Read all data of the response.
	QByteArray response = reply->readAll();
	// Free space.
	reply->deleteLater();
	// Return response
	return response;
}

QByteArray LedDeviceAurora::putJson(QString url, QString json) { 
	// Perfrom request
	QNetworkRequest request(url);
	QNetworkReply* reply = manager->put(request, json.toUtf8());
	// Connect requestFinished signal to quit slot of the loop.
	QEventLoop loop;
	loop.connect(reply, SIGNAL(finished()), SLOT(quit()));
	// Go into the loop until the request is finished.
	loop.exec();
	// Read all data of the response.
	QByteArray response = reply->readAll();
	// Free space.
	reply->deleteLater();
	// Return response
	return response;
}

QByteArray LedDeviceAurora::changeMode(QString host, QString token, QString route) {
	QString url = getUrl(host, token, route);
	QString jsondata(EXT_MODE_STRING); //Enable UDP Mode
	return putJson(url, jsondata);
}

LedDeviceAurora::~LedDeviceAurora()
{
	delete manager;
}

int LedDeviceAurora::write(const std::vector<ColorRgb> & ledValues)
{
    uint udpBufferSize = panelCount * 7 + 1;
	char udpbuffer[udpBufferSize];
	update_num++;
	update_num &= 0xf;

	int i=0;
	int panelCounter = 0;
	udpbuffer[i++] = panelCount;
	for (const ColorRgb& color : ledValues)
	{
		if (i<udpBufferSize) {
			udpbuffer[i++] = panelIds[panelCounter++ % panelCount];
			udpbuffer[i++] = 1; // No of Frames
			udpbuffer[i++] = color.red;
			udpbuffer[i++] = color.green;
			udpbuffer[i++] = color.blue;
			udpbuffer[i++] = 0; // W not set manually
			udpbuffer[i++] = 1; // currently fixed at value 1 which corresponds to 100ms
		}
		if(panelCounter > panelCount) {
			break;
		}
		//printf ("c.red %d sz c.red %d\n", color.red, sizeof(color.red));
	}
	sendto(sockfp, udpbuffer, i, 0, pt->ai_addr, pt->ai_addrlen);

	return 0;
}

int LedDeviceAurora::switchOff()
{
	return 0;
}

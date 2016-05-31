// stl includes
#include <stdexcept>

// Qt includes
#include <QRgb>

// protoserver includes
#include "protoserver/ProtoConnection.h"

ProtoConnection::ProtoConnection(const std::string & a) :
	_socket(),
	_skipReply(false),
	_prevSocketState(QAbstractSocket::UnconnectedState)
{
	QString address(a.c_str());
	QStringList parts = address.split(":");
	if (parts.size() != 2)
	{
		throw std::runtime_error(QString("PROTOCONNECTION ERROR: Wrong address: Unable to parse address (%1)").arg(address).toStdString());
	}
	_host = parts[0];

	bool ok;
	_port = parts[1].toUShort(&ok);
	if (!ok)
	{
		throw std::runtime_error(QString("PROTOCONNECTION ERROR: Wrong port: Unable to parse the port number (%1)").arg(parts[1]).toStdString());
	}

	// try to connect to host
	std::cout << "PROTOCONNECTION INFO: Connecting to Hyperion: " << _host.toStdString() << ":" << _port << std::endl;
	connectToHost();

	// start the connection timer
	_timer.setInterval(5000);
	_timer.setSingleShot(false);

	connect(&_timer,SIGNAL(timeout()), this, SLOT(connectToHost()) );
	_timer.start();
}

ProtoConnection::~ProtoConnection()
{
	_timer.stop();
	_socket.close();
}

void ProtoConnection::setSkipReply(bool skip)
{
	_skipReply = skip;
}

void ProtoConnection::setColor(const ColorRgb & color, int priority, int duration)
{
	proto::HyperionRequest request;
	request.set_command(proto::HyperionRequest::COLOR);
	proto::ColorRequest * colorRequest = request.MutableExtension(proto::ColorRequest::colorRequest);
	colorRequest->set_rgbcolor((color.red << 16) | (color.green << 8) | color.blue);
	colorRequest->set_priority(priority);
	colorRequest->set_duration(duration);

	// send command message
	sendMessage(request);
}

void ProtoConnection::setImage(const Image<ColorRgb> &image, int priority, int duration)
{
	proto::HyperionRequest request;
	request.set_command(proto::HyperionRequest::IMAGE);
	proto::ImageRequest * imageRequest = request.MutableExtension(proto::ImageRequest::imageRequest);
	imageRequest->set_imagedata(image.memptr(), image.width() * image.height() * 3);
	imageRequest->set_imagewidth(image.width());
	imageRequest->set_imageheight(image.height());
	imageRequest->set_priority(priority);
	imageRequest->set_duration(duration);

	// send command message
	sendMessage(request);
}

void ProtoConnection::clear(int priority)
{
	proto::HyperionRequest request;
	request.set_command(proto::HyperionRequest::CLEAR);
	proto::ClearRequest * clearRequest = request.MutableExtension(proto::ClearRequest::clearRequest);
	clearRequest->set_priority(priority);

	// send command message
	sendMessage(request);
}

void ProtoConnection::clearAll()
{
	proto::HyperionRequest request;
	request.set_command(proto::HyperionRequest::CLEARALL);

	// send command message
	sendMessage(request);
}

void ProtoConnection::connectToHost()
{
	// try connection only when 
	if (_socket.state() == QAbstractSocket::UnconnectedState)
	{
	   _socket.connectToHost(_host, _port);
	   //_socket.waitForConnected(1000);
	}
}

void ProtoConnection::sendMessage(const proto::HyperionRequest &message)
{
	// print out connection message only when state is changed
	if (_socket.state() != _prevSocketState )
	{
	  switch (_socket.state() )
	  {
		case QAbstractSocket::UnconnectedState:
		  std::cout << "PROTOCONNECTION INFO: No connection to Hyperion: " << _host.toStdString() << ":" << _port << std::endl;
		  break;

		case QAbstractSocket::ConnectedState:
		  std::cout << "PROTOCONNECTION INFO: Connected to Hyperion: " << _host.toStdString() << ":" << _port << std::endl;
		  break;

		default:
		  //std::cout << "Connecting to Hyperion: " << _host.toStdString() << ":" << _port << std::endl;
		  break;
	  }
	  _prevSocketState = _socket.state();
	}


	if (_socket.state() != QAbstractSocket::ConnectedState)
	{
		return;
	}

	// We only get here if we are connected

	// serialize message (FastWriter already appends a newline)
	std::string serializedMessage = message.SerializeAsString();

	int length = serializedMessage.size();
	const uint8_t header[] = {
		uint8_t((length >> 24) & 0xFF),
		uint8_t((length >> 16) & 0xFF),
		uint8_t((length >>  8) & 0xFF),
		uint8_t((length	  ) & 0xFF)};

	// write message
	int count = 0;
	count += _socket.write(reinterpret_cast<const char *>(header), 4);
	count += _socket.write(reinterpret_cast<const char *>(serializedMessage.data()), length);
	if (!_socket.waitForBytesWritten())
	{
		std::cerr << "PROTOCONNECTION ERROR: Error while writing data to host" << std::endl;
		return;
	}

	if (!_skipReply)
	{
		// read reply data
		QByteArray serializedReply;
		length = -1;
		while (length < 0 && serializedReply.size() < length+4)
		{
			// receive reply
			if (!_socket.waitForReadyRead())
			{
				std::cerr << "PROTOCONNECTION ERROR: Error while reading data from host" << std::endl;
				return;
			}

			serializedReply += _socket.readAll();

			if (length < 0 && serializedReply.size() >= 4)
			{
				// read the message size
				length =
						((serializedReply[0]<<24) & 0xFF000000) |
						((serializedReply[1]<<16) & 0x00FF0000) |
						((serializedReply[2]<< 8) & 0x0000FF00) |
						((serializedReply[3]	) & 0x000000FF);
			}
		}

		// parse reply data
		proto::HyperionReply reply;
		reply.ParseFromArray(serializedReply.constData()+4, length);

		// parse reply message
		parseReply(reply);
	}
}

bool ProtoConnection::parseReply(const proto::HyperionReply &reply)
{
	bool success = false;

	if (!reply.success())
	{
		if (reply.has_error())
		{
			throw std::runtime_error("PROTOCONNECTION ERROR: " + reply.error());
		}
		else
		{
			throw std::runtime_error("PROTOCONNECTION ERROR: No error info");
		}
	}

	return success;
}

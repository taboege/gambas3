/***************************************************************************

  c_webview.h

  (c) Benoît Minisini <g4mba5@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __C_WEBVIEW_H
#define __C_WEBVIEW_H

#include <QUrl>
#include <QWebEnginePage>
#include <QWebEngineView>

//#include "cwebdownload.h"
#include "main.h"

#ifndef __C_WEBVIEW_CPP

/*extern GB_DESC WebViewAuthDesc[];
extern GB_DESC WebViewDownloadsDesc[];*/
extern GB_DESC WebViewDesc[];
extern GB_DESC WebViewHistoryDesc[];
extern GB_DESC WebViewHistoryItemDesc[];

#else

#define THIS      ((CWEBVIEW *)_object)
#define WIDGET    ((MyWebEngineView *)((QT_WIDGET *)_object)->widget)

#endif

class MyWebEngineView : public QWebEngineView
{
	Q_OBJECT

public:
	
	MyWebEngineView(QWidget *parent);

protected:
		
	virtual QWebEngineView *createWindow(QWebEnginePage::WebWindowType type);
};

typedef
	struct 
	{
		QT_WIDGET widget;
		QT_PICTURE icon;
		//QNetworkReply *reply;
		//QAuthenticator *authenticator;
		char *userAgent;
		int history;
		unsigned stopping : 1;
	}
	CWEBVIEW;

#if 0
class CWebView : public QObject
{
  Q_OBJECT

public:

  static CWebView manager;

public slots:

	void iconChanged();
	//void linkClicked(const QUrl &url);
	void loadFinished(bool ok);
	void loadProgress(int progress);
	void loadStarted();
	void selectionChanged();
	void statusBarMessage(const QString &text);
	void titleChanged(const QString &title);
	void linkHovered(const QString &link, const QString &title, const QString &textContent);
	//void downloadRequested(const QNetworkRequest &);
	void frameCreated(QWebFrame *);
	void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
	void urlChanged(const QUrl &);
	void downloadRequested(const QNetworkRequest &);
	void handleUnsupportedContent(QNetworkReply*);
};
#endif
//QNetworkAccessManager *WEBVIEW_get_network_manager();

#endif

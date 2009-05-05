/*
 * imageuploader.cpp
 * Copyright (C) 2009 Alexander Saltykov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "imageuploader.h"

#include <QRegExp>
#include <curl/curl.h>

static size_t cb_write(void *ptr, size_t size, size_t nmemb, void *plasmoid)
{
    ImageUploader *p = static_cast<ImageUploader*>(plasmoid);

    size_t sz = size*nmemb+1;
    char *xml = new char[sz];
    qMemSet(xml, 0, sz);
    qMemCopy(xml, ptr, sz-1);

    QString s(xml);

    delete[] xml;

    QRegExp rx("<image_link>(.*)</image_link>");
    int pos = rx.indexIn(s);
    if ( pos >= 0 ) {
        QString value = rx.cap(1);
        p->setImageUrl(value);
    }
    return size*nmemb;
}

static int cb_progress(void *clientp, double dltotal, double dlnow,
                       double ultotal, double ulnow)
{
    Q_UNUSED(dltotal);
    Q_UNUSED(dlnow);

    ImageUploader *p = static_cast<ImageUploader*>(clientp);
    p->setUploadProgress((ulnow/ultotal)*100);
    return 0;
}

ImageUploader::ImageUploader()
{
    curl_global_init(0);
}

ImageUploader::~ImageUploader()
{
    curl_global_cleanup();
}

QString ImageUploader::uploadFile() const
{
    return m_file;
}

void ImageUploader::setUploadFile(const QString& fileName)
{
    m_file = fileName;
}

void ImageUploader::setImageUrl(const QString& url)
{
    m_url = url;
}

void ImageUploader::setUploadProgress(double percent)
{
    emit uploadProgress(percent);
}

void ImageUploader::run()
{
    m_url.clear();

    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;
    curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "fileupload",
                 CURLFORM_FILE, m_file.toLocal8Bit().constData(), CURLFORM_END);
    curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "xml",
                 CURLFORM_COPYCONTENTS, "yes", CURLFORM_END);

    CURL *h = curl_easy_init();
    curl_easy_setopt(h, CURLOPT_URL, "http://www.imageshack.us/index.php");
    curl_easy_setopt(h, CURLOPT_POST, 1);
    curl_easy_setopt(h, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, cb_write);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(h, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(h, CURLOPT_PROGRESSFUNCTION, cb_progress);
    curl_easy_setopt(h, CURLOPT_PROGRESSDATA, this);

    CURLcode c = curl_easy_perform(h);
    curl_formfree(post);

    if ( c == CURLE_OK ) {
        if ( !m_url.isEmpty() ) {
            emit imageUploaded(m_url);
        }
    } else {
        emit curlError( QString("cURL error: %1").arg(curl_easy_strerror(c)) );
    }

    curl_easy_cleanup(h);
}

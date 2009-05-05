/*
 * imageuploader.h
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

#ifndef IMAGEUPLOADER_H
#define IMAGEUPLOADER_H

#include <QThread>

class QUrl;

class ImageUploader : public QThread
{
    Q_OBJECT

    public:
        ImageUploader();
        ~ImageUploader();

        QString uploadFile() const;
        void setUploadFile(const QString& fileName);

        void setImageUrl(const QString& url);
        void setUploadProgress(double percent);

        void run();
    signals:
        void curlError(const QString& errDesc);
        void uploadProgress(double percent);
        void imageUploaded(const QString& url);
    private:
        QString m_file;
        QString m_url;
};

#endif // IMAGEUPLOADER_H

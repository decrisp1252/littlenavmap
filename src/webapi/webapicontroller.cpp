/*****************************************************************************
* Copyright 2015-2020 Alexander Barthel alex@littlenavmap.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "webapi/webapicontroller.h"
#include "webapi/actionscontrollerindex.h"

#include <QDebug>
#include <QMetaMethod>


WebApiController::WebApiController(QObject *parent, bool verboseParam)
  : QObject(parent), verbose(verboseParam)
{
    qDebug() << Q_FUNC_INFO;
    webApiPathPrefix = "/api";
    registerControllers();
}

void WebApiController::registerControllers(){
    ActionsControllerIndex::registerQMetaTypes();
}

WebApiController::~WebApiController()
{
  qDebug() << Q_FUNC_INFO;
}

WebApiResponse WebApiController::service(WebApiRequest& request)
{
  qDebug() << Q_FUNC_INFO << ":"
           << request.path << ":"
           << request.method << ":"
           << request.body;

  // Create response object
  WebApiResponse response;

  // Set controller and action by string
  QByteArray controllerName = getControllerNameByPath(request.path);
  QByteArray actionName = getActionNameByPath(request.path);

  qDebug() << controllerName << ":" << actionName;

  // Get controller class id
  int id = QMetaType::type(controllerName+"*");

  if (id != 0) {

      // Create controller instance
      const QMetaObject* mo = QMetaType::metaObjectForType(id);
      QObject* controller = mo->newInstance(Q_ARG(QObject*, parent()),Q_ARG(bool, verbose));

      // Invoke action on instance
      bool actionExecuted = QMetaObject::invokeMethod(controller,actionName.data(),Qt::DirectConnection, Q_RETURN_ARG(WebApiResponse,response),Q_ARG(WebApiRequest, request));

      if(!actionExecuted){
          response.status = 400; /* Bad request */
          response.body = "Action not found/failed";
      }

  }else{
      response.status = 400; /* Bad request */
      response.body = "Controller not found";
  }

  return response;

}

QByteArray WebApiController::getControllerNameByPath(QByteArray path){
    QList<QByteArray> list = path.split('/');
    if(list.length() > 1){
        return list[1]+"ActionsController";
    }
};
QByteArray WebApiController::getActionNameByPath(QByteArray path){
    QList<QByteArray> list = path.split('/');
    if(list.length() > 2){
        return list[2]+"Action";
    }
};

/*!
    \mainpage The QtSparql library

    \brief <center>unstable</center>

    \section Introduction

    <b>Description</b>

    QtSparql is a client-side library for accessing RDF stores.

    The query language for RDF stores is <a
    href="http://www.w3.org/TR/rdf-sparql-query/">SPARQL</a>.

    QtSparql takes in SPARQL queries, forwards them to the selected backend, and
    gives back the results of the query.  It can return the results
    asynchronously if the backend supports asynchronous operations.

    QtSparql can connect to different backends.  Currently the following backends
    exist:

    - QTRACKER for accessing <a
      href="http://projects.gnome.org/tracker/">Tracker</a> over D-Bus
    - QTRACKER_DIRECT for accessing <a
      href="http://projects.gnome.org/tracker/">Tracker</a> via direct database
      access and D-Bus. See the \ref trackerdirectspecific "specific useage section"
      for more information
    - QSPARQL_ENDPOINT for accessing online RDF stores, e.g., <a
      href="http://dbpedia.org">DBpedia</a>
    - QVIRTUOSO backend for accessing <a
      href="http://docs.openlinksw.com/virtuoso/">Virtuoso</a>

    <b>List of classes QtSparql API provides:</b>
    <p><table width="100%">
    <tr valign="top" bgcolor="#ffffff"><td><i>Class</i></td><td><i>Description</i></td></tr>

    <tr valign="top" bgcolor="#f0f0f0"><td><b>QSparqlConnection</b></td>
    <td>Interface for accessing an RDF store.</td></tr>

    <tr valign="top" bgcolor="#f0f0f0"><td><b>QSparqlConnectionOptions</b></td>
    <td>Encapsulates options given to QSparqlConnection. Some options are used
    only by some drivers.</td></tr>

    <tr valign="top" bgcolor="#f0f0f0"><td><b>QSparqlError</b></td><td>SPARQL
    error information.</td></tr>

    <tr valign="top" bgcolor="#f0f0f0"><td><b>QSparqlBinding</b></td><td>Handles
    a binding between a SPARQL query variable name and the value of the RDF
    node.</td></tr>

    <tr valign="top" bgcolor="#f0f0f0"><td><b>QSparqlQuery</b></td><td>Means of
    executing and manipulating SPARQL statements.</td></tr>

    <tr valign="top" bgcolor="#f0f0f0"><td><b>QSparqlQueryOptions</b></td>
    <td>Encapsulates query execution options given to QSparqlConnection::exec(const  QSparqlQuery&, const QSparqlQueryOptions&)
    Some options are used only by some drivers.</td></tr>

    <tr valign="top"
    bgcolor="#f0f0f0"><td><b>QSparqlQueryModel</b></td><td>Read-only data model
    for SPARQL result sets.</td></tr>

    <tr valign="top"
    bgcolor="#f0f0f0"><td><b>QSparqlResultRow</b></td><td>Encapsulates a row in
    the results of a query.</td></tr>

    <tr valign="top" bgcolor="#f0f0f0"><td><b>QSparqlResult</b></td><td>Abstract
    interface for accessing the results of an executed QSparqlQuery.</td></tr>

    </table></p>

    \attention The QtSparql library is not yet stable; we make no
    promises about API / ABI compatibility!

    \section gettingstarted Getting started

    The following code snippets demonstrate how to retrieve data from
    a RDF database using QtSparql.

    - Create a QSparqlConnection object specifiying the backend you want to use.
      If necessary, specify the parameters by using QSparqlConnectionOptions and
      passing it to QSparqlConnection.

    E.g. to use tracker:
    \dontinclude simple/main.cpp
    \skipline QSparqlConnection

    E.g. to use DBpedia:
    \dontinclude dbpedia/main.cpp
    \skip QSparqlConnectionOptions
    \until QSPARQL_ENDPOINT

    - Construct a QSparqlQuery with the SPARQL query string.  Specify the query
      type, if needed.

    E.g.
    \dontinclude simple/main.cpp
    \skipline QSparqlQuery

    or

    \dontinclude iteration/main.cpp
    \skip QSparqlQuery insert
    \until InsertStatement

    - Use QSparqlConnection::exec() to execute the query. It returns a
      pointer to QSparqlResult.

    E.g.
    \dontinclude simple/main.cpp
    \skipline QSparqlResult

    - You can then connect to the QSparqlResult::finished() and
      QSparqlResult::dataReady signals. Alternatively for synchronous queries use
      QSparqlResult::syncExec();

    - The QSparqlResult can be iterated over by using the following functions:
      QSparqlResult::first(), QSparqlResult::last(), QSparqlResult::next(),
      QSparqlResult::previous(), QSparqlResult::setPos(). The caller is
      responsible for deleting the QSparqlResult.

    E.g.
    \dontinclude simple/main.cpp
    \skip result->next
    \until toString

    - Data can be retrieved by using QSparqlResult::value().

    The following classes are the most relevant for getting started with QSparql:
    - QSparqlConnection
    - QSparqlQuery
    - QSparqlResult
    - QSparqlQueryModel

    \section querymodels Query models

    The QSparqlQueryModel class provides a convienient, read-only, data model for SPARQL results 
    which can be used to provide data to view classes such as QTableView.

    After creating the model, use QSparqlQueryModel::setQuery() to set the query for the connection,
    header data for the model can also be set using QSparqlQueryModel::setHeaderData().

    E.g.
    \dontinclude querymodel/main.cpp
    \skip model;
    \until setHeaderData

    You can then use this in an a view class by using it's setModel() function.

    E.g.
    \dontinclude querymodel/main.cpp
    \skip *view
    \until model

    It is also easy to implement custom query models by reimplementing QSparqlQueryModel::data(), see
    the querymodel example for an example of this.

    \section connectionoptions Connection options supported by drivers

    QTRACKER_DIRECT driver supports the following connection options:
    - dataReadyInterval (int, default 1), controls the interval for
      emitting the dataReady signal.
    - maxThread (int), sets the maximum number of threads for the
      thread pool to use. If not set a default of number of cores * 2 will
      be used.
    - threadExpiry (int, default 2000), controls the expiry time
      (in milliseconds) of the threads created by the thread pool.

    QENDPOINT driver supports the following connection options:
    - hostName (QString)
    - path (QString)
    - port (int)
    - userName (QString)
    - password (QString)
    - networkAccessManager (QNetworkAccessManager*)
    - proxy (const QNetworkProxy&)
    - custom: "timeout" (int) (for virtuoso endpoints)
    - custom: "maxrows" (int) (for virtuoso endpoints)

    QVIRTUOSO driver supports the following connection options:
    - hostName (QString)
    - port (int)
    - userName (QString)
    - password (QString)
    - databaseName (QString)

    For setting custom options, use QSparqlConnectionOptions::setOption() and
    give the option name as a string, followed by the value.

    Other options can be set using QSparqlConnectionOptions::setOption(), however
    it is preferable to use the convinence functions in QSparqlConnectionOptions,
    as these provide additional error checking.

    \section connectionfeatures Connection features

    The following table describes the QSparclConnection::Feature support of each
    driver. The features can be queried with QSparqlConnection::hasFeature().

    <table>
    <tr>
    <td></td>
    <th>QuerySize</th>
    <th>DefaultGraph</th>
    <th>AskQueries</th>
    <th>ConstructQueries</th>
    <th>UpdateQueries</th>
    <th>SyncExec</th>
    <th>AsyncExec</th>
    </tr>
    <tr>
    <th>QTRACKER</th>
    <td>Yes</td>
    <td>Yes</td>
    <td>Yes</td>
    <td>No</td>
    <td>Yes</td>
    <td>No</td>
    <td>Yes</td>
    </tr>
    <tr>
    <th>QTRACKER_DIRECT</th>
    <td>Yes</td>
    <td>Yes</td>
    <td>Yes</td>
    <td>No</td>
    <td>Yes</td>
    <td>Yes</td>
    <td>No</td>
    </tr>
    <tr>
    <th>QSPARQL_ENDPOINT</th>
    <td>Yes</td>
    <td>Yes</td>
    <td>Yes</td>
    <td>Yes</td>
    <td>Yes</td>
    <td>No</td>
    <td>Yes</td>
    </tr>
    <tr>
    <th>QVIRTUOSO</th>
    <td>Yes</td>
    <td>No</td>
    <td>Yes</td>
    <td>Yes</td>
    <td>Yes</td>
    <td>No (*)</td>
    <td>No</td>
    </tr>
    </table>

    (*) The QVIRTUOSO driver is natively synchronous, but support for syncExec
    directly is not currently implemented.

    \section trackerdirectspecific QTRACKER_DIRECT specific useage

    There are two ways to use the QTRACKER_DIRECT driver, synchronously using
    QSparqlConnection::syncExec(), and asynchronously using QSparqlConnection::exec().
    The result behaviour is different, and supports different features, depending on
    the method used.

    The following table describes the QSparqlResult::Feature support of each method.

    <table>
    <tr>
    <td></td>
    <th>QuerySize</th>
    <th>ForwardOnly</th>
    <th>Sync</th>
    </tr>
    <tr>
    <th>exec()</th>
    <td>Yes</td>
    <td>No</td>
    <td>No</td>
    </tr>
    <tr>
    <th>syncExec()</th>
    <td>No</td>
    <td>Yes</td>
    <td>Yes</td>
    </tr>
    </table>

    When using synchronous execution, it is important to fully use the results returned
    before making another query, either synchronously or asynchronously, by using
    QSparqlResult::next until it returns false. If you fail to do this, any new results
    that may have been added after your original query will not be included in any subsequent
    queries you make.

    \section backendspecific Accessing backend-specific functionalities

    QtSparql doesn't offer backend-specific functionalities.  For that purpose,
    there are separate add-on libraries, e.g., libqtsparql-tracker-extensions.

*/

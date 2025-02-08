using RealLifeApp.DB;
using RealLifeApp.DB.QueryLibrary;
using RealLifeApp.Model;
using RealLifeApp.Net.Device;
using System;
using System.Collections.Generic;
using System.Data.Common;
using System.Data.SqlClient;
using System.Windows.Forms;
using MigraDoc.DocumentObjectModel.IO;
using MigraDoc.Rendering.Forms;
using MigraDocProtocol;
using static RealLifeApp.DB.QueryLibrary.QueryLibraryRefracto;
using System.Globalization;
using System.Drawing.Printing;
using MigraDocProtocol.MigraDocExtensions;
using MigraDoc.Rendering;

namespace RealLifeApp.Print
{
  public class PrintModule
  {
    string varDevSerial;
    string varDevFilename;
    string varDevType;
    string varUser;
    string varDate;
    Dictionary<string, checklistdata> varCheckList;
    List<checklistdata> varCheckList0;
    List<checklistdata> varCheckList1;
    List<checklistdata> varCheckList2;
    List<checklistdata> varCheckList3;
    List<checklistdata> varCheckList4;
    List<checklistdata> varCheckList5;
    List<checklistdata> varCheckList6;

    int devType;
    int devID;
    int devBits;
    CustomDeviceType deviceType;

    private bool doPrintDR6000Chart;
    private bool isKrRico;

    Form owner;

    Dictionary<string, string> contextDescription;
    Dictionary<string, int> contextGroup;

    // TODO Removed "protocol" from groupName
    string[] groupName = { "montage", "general", "func", "calib", "final", "finalize", "box" };
    string[] groupText = { "Montage", "Grundeinstellungen", "Funktionstest", "Justierung", "Endtest", "Abschluss Software", "Geh�use" };

    public PrintModule(TreeNode root, bool chartOk, bool isKrRico, Form owner)
    {
      this.doPrintDR6000Chart = chartOk;
      this.isKrRico = isKrRico;
      this.owner = owner;

      varCheckList = new Dictionary<string, checklistdata>();
      varCheckList0 = new List<checklistdata>();
      varCheckList1 = new List<checklistdata>();
      varCheckList2 = new List<checklistdata>();
      varCheckList3 = new List<checklistdata>();
      varCheckList4 = new List<checklistdata>();
      varCheckList5 = new List<checklistdata>();
      varCheckList6 = new List<checklistdata>();
      contextDescription = new Dictionary<string, string>();
      contextGroup = new Dictionary<string, int>();


      initFromTreeNode(root);
    }


    public bool Preview(IntPtr parentWindowHandle,
                        string devSerial,
                        string devFilename)
    {
      DocumentPreview preview = createPreview(devSerial, devFilename);
      if (preview == null)
      {
        return false;
      }

      var previewDialog = new PreviewDialog(preview);
      previewDialog.Owner = owner;
      previewDialog.ShowDialog(owner);

      return true;
    }

    public bool Print(IntPtr parentWindowHandle,
                      string devSerial,
                      string devFilename,
                      bool previewOnly)
    {
      if (previewOnly == true)
      {
        return Preview(parentWindowHandle, devSerial, devFilename);
      }

      DocumentPreview preview = createPreview(devSerial, devFilename);
      if (preview == null)
      {
        return false;
      }

      int numPages = preview.Renderer.FormattedDocument.PageCount;

      PrinterSettings printerSettings = new PrinterSettings();
      printerSettings.FromPage = 1;
      printerSettings.ToPage = numPages;

      using (PrintDialog dialog = new PrintDialog())
      {
        printerSettings.DefaultPageSettings.Landscape = false;

        dialog.PrinterSettings = printerSettings;
        dialog.AllowSelection = false;
        dialog.AllowSomePages = true;

        if (dialog.ShowDialog() != DialogResult.OK)
        {
          return false;
        }

        printerSettings = dialog.PrinterSettings;

        if (printerSettings.ToPage > numPages)
        {
          printerSettings.ToPage = numPages;
        }

        if (printerSettings.FromPage > numPages)
        {
          printerSettings.FromPage = 1;
        }

        var printDocument = new MigraDocPrintDocument();
        printDocument.PrinterSettings = dialog.PrinterSettings;
        printDocument.Renderer = preview.Renderer;
        printDocument.Print();
      }


      return true;
    }


    public bool PDF(string devSerial,
                    string devFilename,
                    string pdfFilename)
    {
      DocumentPreview preview = createPreview(devSerial, devFilename);
      if (preview == null)
      {
        return false;
      }

      if (pdfFilename == null)
      {
        SaveFileDialog dlg = new SaveFileDialog();
        dlg.RestoreDirectory = true;
        dlg.OverwritePrompt = true;
        dlg.Filter = "PDF|*.pdf";
        DialogResult dlgres = dlg.ShowDialog();
        if (dlgres != DialogResult.OK)
        {
          return false;
        }

        pdfFilename = dlg.FileName;
      }

      PdfDocumentRenderer pdfRenderer = new PdfDocumentRenderer();
      pdfRenderer.DocumentRenderer = preview.Renderer;
      pdfRenderer.Document = preview.Document;

      pdfRenderer.RenderDocument();
      pdfRenderer.Save(pdfFilename);

      return true;
    }

    private DocumentPreview createPreview(string devSerial, string devFilename)
    {
      if (prepareData(devSerial, devFilename) == false)
      {
        return null;
      }

      Protocol protocol = prepareProtocol();
      if (protocol == null)
      {
        return null;
      }

      // This renders the document; takes a while for big documents
      DocumentPreview preview = new DocumentPreview();
      preview.Document = DdlReader.DocumentFromString(protocol.GetDocumentDDL());

      return preview;
    }

    private Protocol prepareProtocol()
    {
      if (contextGroup.Count != contextDescription.Count)
      {
        // Both must be equally sized as they will be acessed by index
        return null;
      }

      ProtocolHeaderData headerData = new ProtocolHeaderData();
      headerData.Title = "Inbetriebnahme-Protokoll (intern)";
      headerData.Serial = varDevSerial;
      headerData.Filename = varDevFilename;
      headerData.DeviceType = varDevType;
      headerData.User = varUser;
      headerData.Date = varDate;

      // Creating a default (A4, portrait) page setup
      ProtocolPageSetup pageSetup = new ProtocolPageSetup();
      pageSetup.RightMargin_cm = 2.5;

      Protocol protocol = new Protocol(headerData, pageSetup);

      //
      // Add the group 'Montage' 
      //
      bool headingPrinted = false;
      foreach (checklistdata item in varCheckList0)
      {
        if (headingPrinted == false)
        {
          protocol.AddHeading1("Montage");
          headingPrinted = true;
        }

        string context = item.context;

        string description = contextDescription[context];
        string user = item.person;
        bool ok = item.ok;

        if (context.EndsWith("montage.vde"))
        {
          protocol.AddHeading1("VDE-Pr�fung");

          var queryLib = new QueryLibraryCommon(devID);
          protocol.AddCheckedParagraph(description, user, ok);
          protocol.AddTable(queryLib.SelectVDETable(),
                            TableDrawMode.Default);
        }
        else
        {
          protocol.AddCheckedParagraph(description, user, ok);
        }
      }

      //
      // Add the group 'Grundeinstellungen' 
      //
      var translations = new Dictionary<string, string>()
      {
        {"general.ipa", "IP-Adresse eingestellt"},
        {"kric.general.ipa", "IP-Adresse eingestellt"},
        {"kric.general.version", "Firmware Version"},
        {"general.params", "Ger�teparameter konfiguriert"},
        {"kric.general.params", "Ger�teparameter konfiguriert"},
        {"general.hellfeld", "Hellfeldmessung durchgef�hrt"},
        {"kric.general.hellfeld", "Hellfeldmessung durchgef�hrt"},
      };

      headingPrinted = false;
      foreach (checklistdata item in varCheckList1)
      {
        if (headingPrinted == false)
        {
          protocol.AddHeading1("Grundeinstellungen");
          headingPrinted = true;
        }

        string context = item.context;
        string value = item.value;
        string description;

        if (!contextDescription.TryGetValue(context, out description))
        {
          description = item.text;
          if (description.Contains("Auckland"))
          {
            // This row contains the Auckland/Cupid version
            double version = double.TryParse(
              value,
              NumberStyles.Any,
              CultureInfo.InvariantCulture,
              out version) ? version : double.NaN;

            // Starting with version 6.00 the device gui board is
            // a Cupid and no Auckland anymore
            if (!double.IsNaN(version) && version >= 6.00)
            {
              // Change the board name in the description
              description = description.Replace("Auckland", "Cupid");
            }
          }
        }

        string user = item.person;
        bool ok = item.ok;

        // Some descriptions should be translated to be shorter.
        // .See dictionary 'translations' above.
        if (translations.ContainsKey(context))
        {
          description = translations[context];
        }

        protocol.AddCheckedParagraph(description, value, user, ok);
        if (context == "kric.general.hellfeld")
        {
          var queryLib = new QueryLibraryRefracto(devID);
          string curveData = queryLib.SelectBrightField(false);
          if (curveData.Length > 0)
          {
            protocol.AddTextParagraph("");

            var curve = new DataPointsXYList(curveData, 1000);
            protocol.AddChart(curve,
                              0, 2050, 200,
                              0, 3000, 0, "0",
                              "10� Pixel", "");
          }
        }
        else if (context == "general.hellfeld")
        {
          var queryLib = new QueryLibraryRefracto(devID);
          string curveData = queryLib.SelectBrightField(true);
          if (curveData.Length > 0)
          {
            protocol.AddTextParagraph("");

            var curve = new DataPointsXYList(curveData);
            protocol.AddChart(curve,
                              0, 2050, 200,
                              0, 5000, 0, "0",
                              "10� Pixel", "");
          }
        }

      }

      //
      // Add the group 'Funktion' 
      //
      headingPrinted = false;
      foreach (checklistdata item in varCheckList2)
      {
        if (headingPrinted == false)
        {
          protocol.AddHeading1("Funktion");
          headingPrinted = true;
        }

        string context = item.context;

        string description = contextDescription[context];
        string user = item.person;
        bool ok = item.ok;

        protocol.AddCheckedParagraph(description, user, ok);
      }

      //
      // Add the group 'Justierung' 
      //
      headingPrinted = false;
      foreach (checklistdata item in varCheckList3)
      {
        if (headingPrinted == false)
        {
          protocol.AddHeading1("Justierung");
          headingPrinted = true;
        }

        string context = item.context;
        string description = contextDescription[context];
        string user = item.person;
        bool ok = item.ok;


        if (context.EndsWith("calib.dr6000.temp"))
        {
          if ((deviceType == CustomDeviceType.DS7500)
            || (deviceType == CustomDeviceType.DS7900))
          {
            user = "(vorkalibrierter Sensor)";
            protocol.AddCheckedParagraph(description, user, ok);
          }
          else
          {
            protocol.AddHeading2("Temperatur");
            protocol.AddCheckedParagraph(description, user, ok);

            var queryLib = new QueryLibraryCommon(devID);
            protocol.AddTable(queryLib.SelectTempAdjustment(),
                              TableDrawMode.Default);

            var adjDate = queryLib.SelectTempAdjustDate();
            if (adjDate == DateTime.MinValue)
            {
              protocol.AddTextParagraph(
                "Datum der Justierung: nicht vorhanden");
            }
            else
            {
              protocol.AddTextParagraph(
                $"Datum der Justierung: {adjDate}");
            }

            if (!isKrRico)
            {
              // For ECO devices also add the temp adjustment coeffs
              protocol.AddTable(queryLib.SelectTempMeasCoeffs(),
                                TableDrawMode.Default);
            }
          }
        }
        else if (context.EndsWith("calib.dr6000.meas"))
        {
          protocol.AddHeading2("Messwert");
          protocol.AddCheckedParagraph(description, user, ok);

          var queryLib = new QueryLibraryRefracto(devID);
          protocol.AddTable(queryLib.SelectMeasAdjustment(),
                            TableDrawMode.SideBySideAuto);

          var adjDate = queryLib.SelectMeasAdjustDate();
          if (adjDate == DateTime.MinValue)
          {
            protocol.AddTextParagraph(
              "Datum der Justierung: nicht vorhanden");
          }
          else
          {
            protocol.AddTextParagraph(
              $"Datum der Justierung: {adjDate}");
          }


          if (!isKrRico)
          {
            // For ECO devices also add the meas adjustment coeffs
            protocol.AddTable(queryLib.SelectMeasCoeffs(),
                              TableDrawMode.Default);
          }

        }
        else if (context.EndsWith("calib.dr6000.caldata"))
        {
          protocol.AddHeading2("Justierdaten");
          description = "Kurve gepr�ft";
          protocol.AddCheckedParagraph(description, user, ok);

          // Get the meas curve with its max. 50 values
          var queryLib = new QueryLibraryRefracto(devID);
          var curve = queryLib.SelectMeasAdjustCurve();

          // Taking this curve and creating an interpolated version
          // of it using the known points and interpolate the missing 
          // ones in-between. This one will have 200 values.
          var curveInterPolated = new DataPointsXYList(2000);
          for (int i = 0; i < curve.Count; i++)
          {
            curveInterPolated.AppendInterpolated((int)curve.PointsX[i] / 100,
                                                 curve.PointsY[i]);
          }

          // TODO
          //curveInterPolated.Extrapolate();

          protocol.AddChart(curveInterPolated,
                            0, 2050, 200,
                            1.0, 1.7, 0, "0.0",
                            "10� Pixel", "nD");
        }
        else if (context.EndsWith("calib.ds7000.pressure"))
        {
          protocol.AddHeading2("Drucksensor");
          protocol.AddCheckedParagraph(description, user, ok);

          var queryLib = new QueryLibraryDensity(devID);
          protocol.AddTable(queryLib.SelectPressureAdjustment(),
                            TableDrawMode.Default);

          var calDate = queryLib.SelectPressureAdjustDate();
          if (calDate.Year > 1970)
          {
            protocol.AddTextParagraph(
              $"Datum der Justierung: {calDate}");
          }
        }
        else if (context.EndsWith("calib.p8000.meas"))
        {
          protocol.AddHeading2("Messwert");
          protocol.AddCheckedParagraph(description, user, ok);

          var queryLib = new QueryLibraryPolari(devID);
          protocol.AddTable(queryLib.SelectMeasAdjustment(),
                            TableDrawMode.Default);

          var calDate = queryLib.SelectMeasAdjustDate();
          if (calDate.Year > 1970)
          {
            protocol.AddTextParagraph(
              $"Datum der Justierung: {calDate}");
          }
        }
        else
        {
          protocol.AddCheckedParagraph(description, user, ok);
        }
      }

      //
      // Add the group 'Endtest' 
      //
      headingPrinted = false;
      foreach (checklistdata item in varCheckList4)
      {
        if (headingPrinted == false)
        {
          protocol.AddHeading1("Endtest");
          headingPrinted = true;
        }

        string context = item.context;
        string description = contextDescription[context];
        string user = item.person;
        bool ok = item.ok;

        protocol.AddCheckedParagraph(description, user, ok);

        if (context.EndsWith("final.meas")) // TODO or only ".temp" ??
        {
          var queryLib = new QueryLibraryCommon(devID);
          protocol.AddTable(queryLib.SelectFinalMeas(),
                            TableDrawMode.Default);
        }
      }

      //
      // Add the group 'Abschluss Software' 
      //
      headingPrinted = false;
      foreach (checklistdata item in varCheckList5)
      {
        if (headingPrinted == false)
        {
          protocol.AddHeading1("Abschluss Software");
          headingPrinted = true;
        }

        string context = item.context;
        string description = contextDescription[context];
        string user = item.person;
        bool ok = item.ok;

        protocol.AddCheckedParagraph(description, user, ok);
      }

      //
      // Add the group 'Geh�use' 
      //
      headingPrinted = false;
      foreach (checklistdata item in varCheckList6)
      {
        if (headingPrinted == false)
        {
          protocol.AddHeading1("Geh�use");
          headingPrinted = true;
        }

        string context = item.context;
        string description = contextDescription[context];
        string user = item.person;
        bool ok = item.ok;

        protocol.AddCheckedParagraph(description, user, ok);
      }

      //
      // Add the final area with date and space for user sign
      //
      string date = DateTime.Now.ToString("dd.MM.yyyy");

      string signText = Environment.NewLine;
      signText += Environment.NewLine;
      signText += Environment.NewLine;
      signText += Environment.NewLine;
      signText += Environment.NewLine;
      signText += Environment.NewLine;
      signText += new string('_', 60);
      signText += Environment.NewLine;
      signText += $"Hamburg, den {date}, Unterschrift {varUser}";

      protocol.AddTextParagraph(signText);

      return protocol;
    }



    /// <summary>
    /// Init from TreeNode
    /// </summary>
    /// <param name="node"></param>
    private void initFromTreeNode(TreeNode node)
    {
      if (node != null)
      {
        List<TreeNode> mainNodes = new List<TreeNode>();
        TreeNode sub = node.FirstNode;
        while (sub != null)
        {
          mainNodes.Add(sub);
          sub = sub.NextNode;
        }

        List<TreeNode> mainNodesSorted = new List<TreeNode>();
        for (int i = 0; i < groupName.Length; i++)
        {
          for (int k = 0; k < mainNodes.Count; k++)
          {
            if (mainNodes[k].Name == groupName[i])
            {
              mainNodesSorted.Add(mainNodes[k]);
              break;
            }
          }
        }

        for (int i = 0; i < mainNodesSorted.Count; i++)
        {
          initFromSubNode(i, mainNodesSorted[i]);
        }
      }
    }

    /// <summary>
    /// Init from Subnode
    /// </summary>
    /// <param name="index"></param>
    /// <param name="node"></param>
    private void initFromSubNode(int index, TreeNode node)
    {
      if (node != null)
      {
        if ((node.Name.Length > 0) && (node.Level > 1))
        {
          string t;
          if ((node.ToolTipText != null) && (node.ToolTipText.Length > 0))
          {
            t = node.ToolTipText;
          }
          else
          {
            t = node.Text;
          }

          addContextDescriptionAndGroup(node.Name, t, index);
        }

        TreeNode sub = node.FirstNode;
        while (sub != null)
        {
          initFromSubNode(index, sub);
          sub = sub.NextNode;
        }
      }
    }

    /// <summary>
    /// Add Context Description and Group
    /// </summary>
    /// <param name="_context"></param>
    /// <param name="_description"></param>
    private void addContextDescriptionAndGroup(string _context, string _description, int _group)
    {
      if (contextDescription.ContainsKey(_context))
      {
        contextDescription[_context] = _description;
      }
      else
      {
        contextDescription.Add(_context, _description);
      }

      if (contextGroup.ContainsKey(_context))
      {
        contextGroup[_context] = _group;
      }
      else
      {
        contextGroup.Add(_context, _group);
      }
    }

    /// <summary>uu
    /// Vorbereiten der Daten zum Druck
    /// </summary>
    /// <param name="_devSerial"></param>
    private bool prepareData(string _devSerial, string _devFileName)
    {
      varDevSerial = _devSerial;
      varDevFilename = _devFileName;
      string prefixKrRico = "";
      if (isKrRico)
      {
        prefixKrRico = "kric.";
      }

      SqlConnection dbconn = new SqlConnection(db.ConnectionString);
      dbconn.Open();

      string sql = "SELECT id," + db.TAB_DEVICE_USERID + " FROM " + db.TAB_DEVICE + " " +
                   "WHERE " + db.TAB_DEVICE_SERIAL + "='" + varDevSerial + "'" +
                   "AND " + db.TAB_DEVICE_FILENAME + "='" + varDevFilename + "'";
      DbDataReader dr = db.QueryDataReader(new SqlCommand(sql, dbconn));
      if (dr == null)
      {
        return false;
      }
      if (!dr.Read())
      {
        dr.Close();
        dbconn.Close();
        return false;
      }
      devID = (int)dr["id"];
      int uid = (int)dr[db.TAB_DEVICE_USERID];
      dr.Close();

      sql = "select " + db.TAB_USER_NACHNAME + "," + db.TAB_USER_VORNAME + " from " + db.TAB_USER + " " +
            "where id=" + uid;
      dr = db.QueryDataReader(new SqlCommand(sql, dbconn));
      if (dr == null)
      {
        dbconn.Close();
        return false;
      }
      if (!dr.Read())
      {
        dr.Close();
        dbconn.Close();
        return false;
      }
      varUser = (string)dr[db.TAB_USER_NACHNAME] + ", " + (string)dr[db.TAB_USER_VORNAME];
      dr.Close();

      sql = "select * from " + db.TAB_PARAMS + " " +
            "where id=" + devID;
      dr = db.QueryDataReader(new SqlCommand(sql, dbconn));
      if (dr == null)
      {
        dbconn.Close();
        return false;
      }
      if (!dr.Read())
      {
        dr.Close();
        dbconn.Close();
        return false;
      }

      devBits = (int)dr[db.TAB_PARAMS_TYP];
      devType = (devBits & 0x7F000000) >> 24;
      deviceType = DeviceBits.Type(devBits);
      dr.Close();

      //if (deviceType == CustomDeviceType.DR6000)
      //{
      //  QueryLibraryRefracto queryLibRefracto = new QueryLibraryRefracto(devID);
      //  queryLibRefracto = new QueryLibraryRefracto(devID);
      //  TargetTable type;
      //  refractoAdjustment = queryLibRefracto
      //    .SelectLatestMeasAdjustment(out refractoAdjustmentDate,
      //                                out type);
      //}

      varDevType = DeviceBits.DeviceName(devBits);
      if (varDevType.Length == 0)
        varDevType = DeviceBits.GroupNameShort(devBits);
      varDate = DateTime.Now.ToShortDateString();

      //---Gruppe "1" Checklistedatenliste vorbereiten---
      int groupIdGeneral = Array.FindIndex(groupName, item => item.EndsWith("general"));
      addCheckListData(groupIdGeneral, devID, dbconn, prefixKrRico + "general.ipa", db.TAB_PARAMS, db.TAB_PARAMS_IPA, typeof(string));
      //addCheckListData(1, devID, dbconn, prefixKrRico + "general.serial", db.TAB_DEVICE, db.TAB_DEVICE_SERIAL, typeof(string));
      //addCheckListData(1, devID, dbconn, prefixKrRico + "general.typ", varDevType);


      addCheckListData(groupIdGeneral, devID, dbconn, prefixKrRico + "general.version",
                       db.TAB_PARAMS, db.TAB_PARAMS_VERSION_AUCKLAND, typeof(string), ".auckland", "Firmware Version Auckland",
                       db.TAB_PARAMS, db.TAB_PARAMS_VERSION_ECO, typeof(string), ".eco", "Firmware Version Eco");


      addCheckListData(groupIdGeneral, devID, dbconn, prefixKrRico + "general.date", db.TAB_PARAMS, db.TAB_PARAMS_TIMESYNC, typeof(DateTime));

      if (devType == 2)
      {
        addCheckListData(groupIdGeneral, devID, dbconn, prefixKrRico + "general.measurement.range", db.TAB_PARAMS, db.TAB_PARAMS_MEAS_RANGE_MIN, db.TAB_PARAMS_MEAS_RANGE_MAX, typeof(double));
      }

      //---f�r alle anderen Gruppen au�er "1" Checklistedatenliste vorbereiten---
      foreach (KeyValuePair<string, string> pair in contextDescription)
        if (contextGroup.ContainsKey(pair.Key))
          if (contextGroup[pair.Key] != 1)
          {
            //if (pair.Key != prefixKrRico + "general.typ" && pair.Key != prefixKrRico + "general.serial")
            //{                                                               // ==> mit Ulf nach Vereinfachung suchen
            checklistdata cld = new checklistdata(pair.Key, contextGroup[pair.Key], pair.Value, false, "");
            varCheckList.Add(pair.Key, cld);
            //}
          }

      //---Checklistedaten aus DB lesen---
      sql = "select * from " + db.TAB_TREE + " where id=" + devID;
      dr = db.QueryDataReader(new SqlCommand(sql, dbconn));
      if (dr == null)
      {
        dbconn.Close();
        return false;
      }
      while (dr.Read())
      {
        string ctx = (string)dr[db.TAB_TREE_CONTEXT];

        if (isKrRico && !ctx.StartsWith(prefixKrRico))
        {
          ctx = prefixKrRico + ctx;
        }

        string person;
        int sta = db.GetTreeStatus(devID, ctx, dbconn, out person);
        if (!varCheckList.ContainsKey(ctx))
        {
          string txt = "";
          int grp = -1;
          if (contextDescription.ContainsKey(ctx))
            txt = contextDescription[ctx];
          if (contextGroup.ContainsKey(ctx))
            grp = contextGroup[ctx];
          varCheckList.Add(ctx, new checklistdata(ctx, grp, txt, sta == 2, person));
        }
        else
        {
          varCheckList[ctx].ok = (sta == 2);
          varCheckList[ctx].person = person;
        }
      }
      dr.Close();

      //---weitere parameter aus DB auslesen---
      if (varCheckList.ContainsKey("finalize.params"))
      {
        checklistdata d = varCheckList["finalize.params"];
        if (d.ok)
        {
          sql = "select " + db.TAB_CONFIG_TIMESTAMP + " from " + db.TAB_CONFIG + " " +
                "where (id=" + devID + ")and(" + db.TAB_CONFIG_TYP + "=" + (int)db.ConfigType.DeviceParametersBinary + ")";
          dr = db.QueryDataReader(new SqlCommand(sql, dbconn));
          if (dr != null)
          {
            if (dr.Read())
            {
              d.value = ((DateTime)dr[db.TAB_CONFIG_TIMESTAMP]).ToString();
            }
            dr.Close();
          }
        }
      }

      if (varCheckList.ContainsKey(prefixKrRico + "finalize.db"))
      {
        checklistdata d = varCheckList[prefixKrRico + "finalize.db"];
        if (d.ok)
        {
          sql = "select " + db.TAB_CONFIG_TIMESTAMP + " from " + db.TAB_CONFIG + " " +
                "where (id=" + devID + ")and(" + db.TAB_CONFIG_TYP + "=" + (int)db.ConfigType.DeviceDatabaseBinary + ")";
          dr = db.QueryDataReader(new SqlCommand(sql, dbconn));
          if (dr != null)
          {
            if (dr.Read())
            {
              d.value = ((DateTime)dr[db.TAB_CONFIG_TIMESTAMP]).ToString();
            }
            dr.Close();
          }
        }
      }

      //---prepare group 0 data---
      foreach (KeyValuePair<string, checklistdata> pair in varCheckList)
        if (pair.Value.group == 0)
          varCheckList0.Add(new checklistdata(pair.Value));

      //---prepare group 1 data---
      foreach (KeyValuePair<string, checklistdata> pair in varCheckList)
        if (pair.Value.group == 1)
          if (pair.Key != "general.version")
            varCheckList1.Add(new checklistdata(pair.Value));

      //---prepare group 2 data---
      foreach (KeyValuePair<string, checklistdata> pair in varCheckList)
        if (pair.Value.group == 2)
          varCheckList2.Add(new checklistdata(pair.Value));

      //---prepare group 3 data---
      foreach (KeyValuePair<string, checklistdata> pair in varCheckList)
        if (pair.Value.group == 3)
          varCheckList3.Add(new checklistdata(pair.Value));

      //---prepare group 4 data---
      foreach (KeyValuePair<string, checklistdata> pair in varCheckList)
        if (pair.Value.group == 4)
          varCheckList4.Add(new checklistdata(pair.Value));

      //---prepare group 5 data---
      foreach (KeyValuePair<string, checklistdata> pair in varCheckList)
        if (pair.Value.group == 5)
          varCheckList5.Add(new checklistdata(pair.Value));

      //---prepare group 6 data---
      foreach (KeyValuePair<string, checklistdata> pair in varCheckList)
        if (pair.Value.group == 6)
          varCheckList6.Add(new checklistdata(pair.Value));

      dbconn.Close();

      return true;
    }


    /// <summary>
    /// Add checklist data
    /// </summary>
    /// <param name="_group"></param>
    /// <param name="_devID"></param>
    /// <param name="_dbconn"></param>
    /// <param name="_context"></param>
    private void addCheckListData(int _group, int _devID, SqlConnection _dbconn, string _context)
    {
      addCheckListData(_group, _devID, _dbconn, _context, null, null, typeof(int));
    }

    /// <summary>
    /// Add checklist data with value
    /// </summary>
    /// <param name="_group"></param>
    /// <param name="_devID"></param>
    /// <param name="_dbconn"></param>
    /// <param name="_context"></param>
    /// <param name="_table"></param>
    /// <param name="_field"></param>
    /// <param name="_type"></param>
    private void addCheckListData(int _group, int _devID, SqlConnection _dbconn, string _context,
                                  string _table, string _field, Type _type)
    {
      checklistdata data = new checklistdata();
      data.group = _group;
      data.ok = (db.GetTreeStatus(_devID, _context, _dbconn, out data.person) >= 2);
      data.value = readFromDB(_devID, _dbconn, _table, _field, _type);
      data.context = _context;
      if (contextDescription.ContainsKey(_context))
        data.text = contextDescription[_context];
      varCheckList.Add(_context, data);
    }

    /// <summary>
    /// Add checklist data with value
    /// </summary>
    /// <param name="_group"></param>
    /// <param name="_devID"></param>
    /// <param name="_dbconn"></param>
    /// <param name="_context"></param>
    /// <param name="_table"></param>
    /// <param name="_field1"></param>
    /// <param name="_field2"></param>
    /// <param name="_type"></param>
    private void addCheckListData(int _group, int _devID, SqlConnection _dbconn, string _context,
                                  string _table, string _field1, string _field2, Type _type)
    {
      checklistdata data = new checklistdata();
      data.group = _group;
      data.ok = (db.GetTreeStatus(_devID, _context, _dbconn, out data.person) >= 2);
      data.value = readFromDB(_devID, _dbconn, _table, _field1, _type) + " - " + readFromDB(_devID, _dbconn, _table, _field2, _type);
      if (contextDescription.ContainsKey(_context))
        data.text = contextDescription[_context];
      varCheckList.Add(_context, data);
    }

    /// <summary>
    /// Add checklist data
    /// </summary>
    /// <param name="_group"></param>
    /// <param name="_devID"></param>
    /// <param name="_dbconn"></param>
    /// <param name="_context"></param>
    /// <param name="_table1"></param>
    /// <param name="_field1"></param>
    /// <param name="_type1"></param>
    /// <param name="_postfix1"></param>
    /// <param name="_text1"></param>
    /// <param name="_table2"></param>
    /// <param name="_field2"></param>
    /// <param name="_type2"></param>
    /// <param name="_postfix2"></param>
    /// <param name="_text2"></param>
    private void addCheckListData(int _group, int _devID, SqlConnection _dbconn, string _context,
                                  string _table1, string _field1, Type _type1, string _postfix1, string _text1,
                                  string _table2, string _field2, Type _type2, string _postfix2, string _text2)
    {
      string person;
      bool ok = (db.GetTreeStatus(_devID, _context, _dbconn, out person) >= 2);

      checklistdata data;

      if (_context.StartsWith("kric"))
      {
        // This is a Kr-Rico device. So it certainly has a new 
        // introduced firmware version descriptor. This item is
        // added now.

        var queryLib = new QueryLibraryCommon(devID);
        string fwVersion = queryLib.SelectFirmwareVersion();
        if (string.IsNullOrEmpty(fwVersion))
        {
          fwVersion = "unbekannt";
        }

        string postfix = ".firmware";

        data = new checklistdata();
        data.group = _group;
        data.ok = ok;
        data.person = person;
        data.value = fwVersion;
        data.text = "Firmware Version";
        data.context = _context + postfix;
        varCheckList.Add(_context + postfix, data);

        // At the moment Kr-Rico devices always have these boards
        _text1 = "Version CUPID";
        _text2 = "Version KMU";
      }

      data = new checklistdata();
      data.group = _group;
      data.ok = ok;
      data.person = person;
      data.value = readFromDB(_devID, _dbconn, _table1, _field1, _type1);
      data.text = _text1;
      data.context = _context + _postfix1;
      varCheckList.Add(_context + _postfix1, data);

      data = new checklistdata();
      data.group = _group;
      data.ok = ok;
      data.person = person;
      data.value = readFromDB(_devID, _dbconn, _table2, _field2, _type2);
      data.text = _text2;
      data.context = _context + _postfix1;
      varCheckList.Add(_context + _postfix2, data);
    }

    /// <summary>
    /// Add checklist data with value
    /// </summary>
    /// <param name="_group"></param>
    /// <param name="_devID"></param>
    /// <param name="_dbconn"></param>
    /// <param name="_context"></param>
    /// <param name="_table"></param>
    /// <param name="_field"></param>
    /// <param name="_type"></param>
    private void addCheckListData(int _group, int _devID, SqlConnection _dbconn, string _context, string _value)
    {
      checklistdata data = new checklistdata();
      data.group = _group;
      data.ok = (db.GetTreeStatus(_devID, _context, _dbconn, out data.person) >= 2);
      data.value = _value;
      if (contextDescription.ContainsKey(_context))
        data.text = contextDescription[_context];
      varCheckList.Add(_context, data);
    }

    /// <summary>
    /// Read Value from database table
    /// </summary>
    /// <param name="_devID"></param>
    /// <param name="_dbconn"></param>
    /// <param name="_table"></param>
    /// <param name="_field"></param>
    /// <returns></returns>
    private static string readFromDB(int _devID, SqlConnection _dbconn, string _table, string _field, Type _type)
    {
      if (_table == null)
        return "";
      if (_field == null)
        return "";
      string sql = "select " + _field + " from " + _table + " where id=" + _devID;
      DbDataReader dr = db.QueryDataReader(new SqlCommand(sql, _dbconn));
      if (dr == null)
        return "";
      object obj = DBNull.Value;
      if (dr.Read())
        obj = dr[_field];
      dr.Close();
      if (obj is DBNull)
        return "";
      if (_type == typeof(string))
        return (string)obj;
      else if (_type == typeof(DateTime))
        return ((DateTime)obj).ToString();
      else if (_type == typeof(int))
        return ((int)obj).ToString();
      else if (_type == typeof(long))
        return ((long)obj).ToString();
      else if (_type == typeof(double))
        return ((double)obj).ToString();
      else
        return "";
    }

    class checklistdata
    {
      internal string text;
      internal string person;
      internal string value;
      internal string context;
      internal int group;
      internal bool ok;

      internal checklistdata()
      {
        text = "";
        person = "";
        ok = false;
        value = "";
        group = 0;
        context = "";
      }

      internal checklistdata(string _context, int _group, string _text, bool _ok, string _person)
      {
        text = _text;
        person = _person;
        ok = _ok;
        value = "";
        group = _group;
        context = _context;
      }

      internal checklistdata(string _context, int _group, string _text, bool _ok, string _person, string _value)
      {
        text = _text;
        person = _person;
        ok = _ok;
        value = _value;
        group = _group;
        context = _context;
      }

      internal checklistdata(checklistdata _other)
      {
        text = _other.text;
        person = _other.person;
        ok = _other.ok;
        value = _other.value;
        group = _other.group;
        context = _other.context;
      }
    }




  }
  
  // Use this added line to measure the search performance
}

// Decompile every AutoRechargeServer / PidRechargeController method of
// libautomatic_recharge.so (mower firmware v6.0.2) to
// research/ghidra_output/auto_recharge_server_decompiled.c.
//
//   analyzeHeadless <proj_dir> autorecharge \
//     -import research/firmware/mower_firmware_v6.0.2/install/automatic_recharge/lib/libautomatic_recharge.so \
//     -scriptPath research/ghidra-scripts -postScript DecompAutoRecharge.java <output.c>
//@category Novabot
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.util.task.ConsoleTaskMonitor;
import java.io.PrintWriter;

public class DecompAutoRecharge extends GhidraScript {
  public void run() throws Exception {
    String[] args = getScriptArgs();
    String out = args.length > 0 ? args[0] : "auto_recharge_server_decompiled.c";
    String[] owners = {"AutoRechargeServer", "PidRechargeController"};
    FunctionManager fm = currentProgram.getFunctionManager();
    DecompInterface decomp = new DecompInterface();
    decomp.openProgram(currentProgram);
    PrintWriter w = new PrintWriter(out, "UTF-8");
    w.println("// Ghidra decompile of libautomatic_recharge.so (auto_recharge_server, mower fw v6.0.2)");
    w.println("// Functions: AutoRechargeServer::*, PidRechargeController::*, StringWorkStatus");
    FunctionIterator it = fm.getFunctions(true);
    int n = 0;
    while (it.hasNext()) {
      Function f = it.next();
      String full = f.getName(true);
      boolean m = full.startsWith("StringWorkStatus");
      for (String o : owners) { if (full.startsWith(o + "::")) { m = true; break; } }
      if (!m) continue;
      w.println("\n// ==================== " + full + " @ " + f.getEntryPoint() + " ====================");
      DecompileResults res = decomp.decompileFunction(f, 300, new ConsoleTaskMonitor());
      if (res != null && res.decompileCompleted()) w.println(res.getDecompiledFunction().getC());
      else w.println("// decompile FAILED: " + (res == null ? "null" : res.getErrorMessage()));
      n++;
      println("decompiled " + full);
    }
    w.println("\n// ==== DONE: " + n + " functions");
    w.close();
    println("==== DONE DecompAutoRecharge: " + n + " functions -> " + out);
  }
}

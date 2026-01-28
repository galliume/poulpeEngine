module;

extern "C" {
  #include <tcl.h>
  #include <tclDecls.h>
  #include <tk.h>

  inline void PLP_Tcl_SetResult(Tcl_Interp *interp, char *result, Tcl_FreeProc *freeProc) {
    Tcl_SetResult(interp, result, freeProc);
  }

  inline int PLP_Tcl_Eval(Tcl_Interp *interp, const char *script) {
    return Tcl_Eval(interp, script);
  }

  inline char* PLP_Tcl_GetStringResult(Tcl_Interp *interp) {
    return Tcl_GetStringResult(interp);
  }
}

export module Engine.Core.TclTk;

#undef TCL_OK
#undef TCL_ERROR
#undef TCL_RETURN
#undef TCL_BREAK
#undef TCL_CONTINUE
#undef TCL_STATIC
#undef TCL_VOLATILE
#undef TCL_DYNAMIC
#undef TCL_EVAL_DIRECT

export
{
  using ::Tcl_Interp;
  using ::Tcl_Obj;
  using ::Tk_Window;
  using ::ClientData;
  using ::Tcl_FreeProc;
  using ::Tcl_FindExecutable;
  using ::Tcl_CreateInterp;
  using ::Tcl_Init;
  using ::Tk_Init;
  using ::Tcl_CreateObjCommand;
  using ::Tcl_GetString;
  using ::Tcl_NewStringObj;
  using ::Tcl_SetObjResult;
  using ::Tcl_EvalEx;
  using ::Tk_MainWindow;
  using ::Tcl_GetIntFromObj;
  using ::Tcl_NewListObj;
  using ::Tk_MainLoop;
  using ::Tcl_DeleteInterp;
  using ::Tcl_DStringResult;
  using ::Tcl_Finalize;
  using ::Tcl_GetStringFromObj;
  using ::Tcl_GetObjResult;
  using ::Tcl_EvalFile;
  using ::Tcl_ListObjAppendElement;
  using ::Tcl_EvalObjEx;
  using ::Tcl_IncrRefCount;
  using ::Tcl_DecrRefCount;
}

#undef Tcl_SetResult
#undef Tcl_Eval
#undef Tcl_GetStringResult

export inline void Tcl_SetResult(Tcl_Interp *interp, char *result, Tcl_FreeProc *freeProc) {
  PLP_Tcl_SetResult(interp, result, freeProc);
}

export inline int Tcl_Eval(Tcl_Interp *interp, const char *script) {
  return PLP_Tcl_Eval(interp, script);
}

export inline char* Tcl_GetStringResult(Tcl_Interp *interp) {
  return PLP_Tcl_GetStringResult(interp);
}

export {

  inline constexpr int TCL_OK = 0;
  inline constexpr int TCL_ERROR = 1;
  inline constexpr int TCL_RETURN = 2;
  inline constexpr int TCL_BREAK = 3;
  inline constexpr int TCL_CONTINUE = 4;
  inline constexpr int TCL_EVAL_DIRECT = 0x040000;

  inline Tcl_FreeProc* const TCL_STATIC = nullptr;
  inline Tcl_FreeProc* const TCL_VOLATILE = reinterpret_cast<Tcl_FreeProc*>(-1);
  inline Tcl_FreeProc* const TCL_DYNAMIC = reinterpret_cast<Tcl_FreeProc*>(3);
}

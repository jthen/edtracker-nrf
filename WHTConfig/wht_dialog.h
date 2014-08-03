#pragma once

class WHTDialog
{
private:
	HWND		hDialog;
	WHTDevice	device;

	void ReadConfigFromDevice();
	void SendConfigToDevice();

	void SetCtrlText(int ctrl_id, const std::wstring& text);
	void SetCtrlTextFloat(int ctrl_id, float flt)
	{
		SetCtrlText(ctrl_id, int2flt(flt));
	}

	float GetCtrlTextFloat(int ctrl_id);

	void SetRadioState(int ctrl_id, bool new_state);
	bool GetRadioState(int ctrl_id)
	{
		return SendMessage(GetDlgItem(hDialog, ctrl_id), BM_GETCHECK, 0, 0) == BST_CHECKED;
	}

	void SetCheckState(int ctrl_id, bool new_state)
	{
		CheckDlgButton(hDialog, ctrl_id, new_state ? BST_CHECKED : BST_UNCHECKED);
	}

	bool GetCheckState(int ctrl_id)
	{
		return IsDlgButtonChecked(hDialog, ctrl_id) == BST_CHECKED;
	}

	void OnCommand(int ctrl_id);

	void ConnectedUI();
	void DisconnectedUI();

public:
	explicit WHTDialog(HWND hDlg);
	~WHTDialog();

	static BOOL CALLBACK MyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};
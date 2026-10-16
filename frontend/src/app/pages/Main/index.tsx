import { useLayoutEffect } from 'react';
import { TitleBar } from './TitleBar';
import { Workspace } from './Workspace';
import './workspace.css';

export function Main() {
  useLayoutEffect(() => { window.bigoMain.windows.ready(); }, []);
  return <>
    <TitleBar />
    <Workspace />
  </>;
}

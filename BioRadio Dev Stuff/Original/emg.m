%%%%%
% MATLAB GUI to fly a quadrotor using EMG state classification.
%%%%%

function varargout = emg(varargin)
% EMG MATLAB code for emg.fig
%      EMG, by itself, creates a new EMG or raises the existing
%      singleton*.
%
%      H = EMG returns the handle to a new EMG or the handle to
%      the existing singleton*.
%
%      EMG('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in EMG.M with the given input arguments.
%
%      EMG('Property','Value',...) creates a new EMG or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before emg_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to emg_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help emg

% Last Modified by GUIDE v2.5 08-Mar-2013 02:25:37

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @emg_OpeningFcn, ...
                   'gui_OutputFcn',  @emg_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before emg is made visible.
function emg_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to emg (see VARARGIN)

global h fly baselineThresh ...
       butterC1 butterC2 L Nsh H
%%%%% FILTER STUFF
%     [b,a]=butter(5,[50*2/1000 70*2/1000],'stop'); %defines 5th order Butterworth filter with 6Hz cutoff frequency
[butterC1,butterC2]=butter(5,140*2/1000,'high'); %defines 5th order Butterworth filter with 6Hz cutoff frequency
%     [b,a]=butter(5,[110*2/1000 130*2/1000],'stop'); %defines 5th order Butterworth filter with 6Hz cutoff frequency

stimfreq = 20;
srate    = 1000;
L = srate/stimfreq;
Nsh = 15;
d = fdesign.comb('notch','L,BW,GBW,Nsh',L,3,-35,Nsh,srate);
% d = fdesign.comb('notch','L,BW,GBW,Nsh',1000/20,1,-3,4,1000);
H = design(d);
%%%%%

% Choose default command line output for emg
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);
% h = actxserver('WScript.Shell');
% h.Run('c:/autoflight/AutoFlight.exe');
baselineThresh = [];
fly = false;


% UIWAIT makes emg wait for user response (see UIRESUME)
% uiwait(handles.figure1);

% --- Outputs from this function are returned to the command line.
function varargout = emg_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in setNullbtn.
function setNullbtn_Callback(hObject, eventdata, handles)
% hObject    handle to setNullbtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global nullStarted timeNullStarted nullDuration
nullStarted = true;
timeNullStarted = 0;
nullDuration = 2;


% --- Executes on button press in startTrialbtn.
function startTrialbtn_Callback(hObject, eventdata, handles)
% hObject    handle to startTrialbtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global trialStarted recordStart
trialStarted = true;
recordStart = false;

% --- Executes on selection change in actionMenu.
function actionMenu_Callback(hObject, eventdata, handles)
% hObject    handle to actionMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns actionMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from actionMenu


% --- Executes during object creation, after setting all properties.
function actionMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to actionMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function trialNumText_Callback(hObject, eventdata, handles)
% hObject    handle to trialNumText (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of trialNumText as text
%        str2double(get(hObject,'String')) returns contents of trialNumText as a double


% --- Executes during object creation, after setting all properties.
function trialNumText_CreateFcn(hObject, eventdata, handles)
% hObject    handle to trialNumText (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in startDAQbtn.
function startDAQbtn_Callback(hObject, eventdata, handles)
% hObject    handle to startDAQbtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of startDAQbtn

btnState = get(hObject,'Value');
global s data time curHandles ch1plot ch2plot session nullStarted timeNullStarted trialStarted
global signals non_filtData

%Initialize global vars
curHandles = handles;
nullStarted = false;
timeNullStarted = 0;
trialStarted = 0;

signals = cell(1,4);

if btnState
    ch1plot = plot(handles.axes1,0,0);
    ch2plot = plot(handles.axes2,0,0);
    set(hObject,'String', 'Stop DAQ');
    data = [];
    time = [];
    non_filtData = [];
    daqreset
    session = false;
    if session
        s = daq.createSession('ni');
        s.addAnalogInputChannel('Dev3',0,'Voltage');
        s.addAnalogInputChannel('Dev3',1,'Voltage');
        s.IsContinuous = true;
        s.addlistener('DataAvailable', @getMoreData);
        s.NotifyWhenDataAvailableExceeds = s.Rate/10; %getMoreData runs every .1s
        s.startBackground();
    else
        numChannels = 2;
%         s = analoginput('nidaq','Dev1'); % Check Dev1 with >>daqhwinfo('nidaq');
        s = analoginput('iotdaq','1');
        s.InputType = 'SingleEnded';
%         addchannel(s,1:numChannels);     % Add numChannels input channels to analog input
        addchannel(s,[0 2]);     % Add numChannels input channels to analog input

        % Initialize analog input channels
        s.Channel(1).ChannelName = 'arm';
        s.Channel(2).ChannelName = 'arm';

        s.Channel.InputRange = [-10 10];
        rate = 1000; % Sample at 1000 Hz (shouldn't change)
        period = .1;
        duration = inf;  % Record for desired amount of seconds
        set(s,'SampleRate',rate);                  % Sample at 1KHz
        set(s,'SamplesPerTrigger', duration*rate); % Continuous processing
        set(s,'TimerPeriod',period);               %repeat timer function every period
        set(s,'TimerFcn',{@getMoreData});           %call timerevent every period
        start(s);
    end
else
    if session
        s.stop();
        delete(s);
    else
        stop(s);
    end
    set(hObject,'String', 'Start DAQ');
end

function getMoreData(src,event)
    global data time curHandles session ch1plot ch2plot
    global nullStarted timeNullStarted nullDuration baselineThresh
    global trialStarted recordStart trialData
    global signals
    global fly fclass curClass trFeats trClass prevKey mouse h
    global Nsh L H butterC1 butterC2 non_filtData
    
    %%
    % PLOTTING THE DATA
    %
    
    ch1axes = curHandles.axes1;
    ch2axes = curHandles.axes2;
    if session
        newData = event.Data;
        newTime = event.TimeStamps;
        rate = src.Rate;
    else
        if src.SamplesAvailable > 0
            [newData,newTime] = getdata(src,src.SamplesAvailable);
            rate = src.SampleRate;
        else
            return
        end
    end
    
    
    
    
    
    
    
    
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%% ADD IN THE NEW FILTERS %%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    
    
    % % % %     %OLD STUFF
    % % % %     % Notch will go here
    % % % % %     Wo = 60/(300/2);
    % % % % %     BW = Wo/35;
    % % % %     [b,a] = butter(5,80*2/1000,'high'); % iirnotch(Wo,BW); %
    % % % %     for i = 1:size(newData,2)
    % % % %        newData(:,i) = filtfilt(b,a,newData(:,i)); %runs filter
    % % % %     end
    
    %Step 1: Butter the input data
    %Still need to do a filter on the original data
    %NOTE: EMGraw technically = EMGraw+butter at this point
    newData = filtfilt(butterC1,butterC2,newData); %runs filter
    
    %OLD STUFF
    non_filtData = [non_filtData; newData];
    time = [time; newTime];
    
    % Keep the last 5s of data
    if isempty(time) || newTime(end) < 5
        
        %Plots EMG if not flying
        set(ch1axes,'XLim',[time(1) time(end)],'XTick', time(1):.5:time(end),'YLim',[-4 4]);
        set(ch2axes,'XLim',[time(1) time(end)],'XTick', time(1):.5:time(end),'YLim',[-4 4]);
        set(ch1plot,'xdata',time,'ydata',non_filtData(:,1));
        set(ch2plot,'xdata',time,'ydata',non_filtData(:,2),'Color',[0 .5 0]);
        drawnow;
        
        data = non_filtData;
    else
        %Step Bounus: Add a blanking window
        for i = 1:size(newData,2)
            [~, index] = max(newData(:,i));
            
            %Mainly going to be this case
            if(index > 2 && index < size(newData,1)-1)
                newData(index-2:index+2,i) = 0;
            elseif(index <= 2) %Just in case its the first or second term
                newData(index:index+2,i) = 0;
            else %Just in case its the last or penultimate term
                newData(index-2:index,i) = 0;
            end
        end
        
        %Step 0: Create some constants
        padding = Nsh*L*2;
        
        %Step 3: Append some of the old butter to the new butter so we get ~500 ms
        amountToUse = 500; %Amount of data to use for comb filter [ms]
        currData = [non_filtData(end-(amountToUse-size(newData,1))+1:end,:); newData]; %X
        
        %Step 4: Comb the ~500 ms of data and take only 100 (inner) ms of it
        %         currEMG = padarray(currEMG(:,i),[padding 0]);%,'symmetric','both');          %Xpad
        currData = filtfilt(H.Numerator, H.Denominator, padarray(currData,[padding 0])); %Xfilt
        currData = currData(end-padding-200+1:end-padding-100,:);                        %Xfilt
        curr_filtData  = currData;
        
        data = [data; curr_filtData];
        
        %Plots EMG if not flying
        set(ch1axes,'XLim',[time(end-200+1) time(end-100)],'XTick', time(end-200+1):.5:time(end-100),'YLim',[-4 4]);
        set(ch2axes,'XLim',[time(end-200+1) time(end-100)],'XTick', time(end-200+1):.5:time(end-100),'YLim',[-4 4]);
        set(ch1plot,'xdata',time(end-200+1:end-100),'ydata',curr_filtData(:,1));
        set(ch2plot,'xdata',time(end-200+1:end-100),'ydata',curr_filtData(:,2),'Color',[0 .5 0]);
        drawnow;
    end
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%         
    %%
    % GETTING BASELINE THRESHOLD
    %
    
    % If get null threshold button clicked
    if nullStarted
        beep;
        timeNullStarted = time(end);
        nullStarted = false;
        set(curHandles.statusText,'String', 'Recording...');
    end
    
    % Get nullDuration amount of data for baseline thresh calculation
    if timeNullStarted ~= 0 && time(end)-timeNullStarted >= nullDuration
        set(curHandles.statusText,'String', '');
        nullData = data(end-nullDuration*rate+1:end,:);
        baseMult = str2double(get(curHandles.baseMultText,'String'));
        baselineThresh = baseMult*sqrt(mean(nullData.^2))
        timeNullStarted = 0;
        beep;
    end
    
    
    %%
    % GET DATA FOR TRIAL
    %
    
    if trialStarted
%         rms = sqrt(mean(data(end-.200*rate+1:end,:).^2));
        rms = mean(abs(data(end-.200*rate+1:end,:)));
        actionNum = get(curHandles.actionMenu, 'Value');
        if ~recordStart
            set(curHandles.statusText,'String', 'Waiting...');
            if sum(rms > baselineThresh) > 0
                set(curHandles.statusText,'String', 'Recording...');
                rms
                beep;
                recordStart = true;
                trialData = [];
            end
        end
        if recordStart
            trialData = [trialData; newData];
            if sum(rms > baselineThresh) == 0 %Stop recording
                beep;
                recordStart = false;
                trialNum = str2double(get(curHandles.trialNumText,'String'))
                signals{actionNum}{trialNum} = trialData;
                trialData = [];
                if trialNum < 5
                    set(curHandles.trialNumText,'String',num2str(trialNum+1));
                else
                    set(curHandles.trialNumText,'String','1');
                    set(curHandles.statusText,'String', '');
                    trialStarted = false;
                end
            end
        end
    end

    if fly
        import java.awt.event.KeyEvent;
        
        fclass = circshift(fclass,[1 -1]);
        signal = data(end-round(.200*rate)+1:end,:);
        numChannels = size(signal,2);
        
        %Detrend
        X = zeros(length(signal)-7,numChannels);
        for j=8:length(signal)
            X(j-7,:) = signal(j,:) - mean(signal(j-7:j,:));
        end
        
%         cNames = {'null' 'left' 'right' 'forward' 'takeoff'};
        cNames = {'null' 'up' 'down'};
      
        %Extract features
        numFeat = 4;
        feat = zeros(1,numChannels*numFeat);
        rms = sqrt(mean(X.^2)); %RMS
    
        if sum(rms > baselineThresh)==0
            fclass(10) = 1;
        else
            wl = sum(abs(diff(X))); %waveform length

            for k=1:numChannels
                offset = numFeat*(k-1);
                feat(offset+1) = rms(k);
                feat(offset+2) = wl(k);

                for l=1:length(X)-1
                    x = X(l,k);
                    xf = X(l+1,k);

                    %Trial Zero Crossings
                    if ((x>0 && xf<0) || (x<0 && xf>0)) && abs(x-xf)>=.01
                        feat(offset+3) = feat(offset+3)+1;
                    end

                    %Trial Slope Sign Changes
                    if l>1
                        xp = X(l-1,k);
                        if ((x>xp && x>xf) || (x<xp && x<xf)) && (abs(x-xf)>=.01 || abs(x-xp)>=.01)
                            feat(offset+4) = feat(offset+4)+1;
                        end
                    end
                end
            end
        
            fcName = classify(feat,trFeats,trClass);
%             disp(fcName)
            fclass(10) = find(strcmp(cNames,fcName));
        end  
        prevClass = curClass;
        if fclass(9)==1 && fclass(10)==1
            curClass = cNames{1}; %null
%             disp(curClass)
        else
            %Get majority class
            curClass = cNames{mode(fclass)};
%             disp(curClass)
        end
        
        set(curHandles.statusText,'String',curClass);
%         if strcmp(curClass, 'forward') && ~strcmp(prevClass, 'forward')
%             mouse.keyRelease(prevKey);
%             mouse.keyPress(KeyEvent.VK_UP);
%             prevKey = KeyEvent.VK_UP;
%         elseif strcmp(curClass, 'takeoff') && ~strcmp(prevClass, 'takeoff')
%             mouse.keyRelease(prevKey);
%             mouse.keyPress(KeyEvent.VK_SHIFT);
%             prevKey = KeyEvent.VK_SHIFT;
%         elseif strcmp(curClass, 'left') && ~strcmp(prevClass, 'left')
%             mouse.keyRelease(prevKey);
%             mouse.keyPress(KeyEvent.VK_Z);
%             prevKey = KeyEvent.VK_Z;
%         elseif strcmp(curClass, 'right') && ~strcmp(prevClass, 'right')
%             mouse.keyRelease(prevKey);
%             mouse.keyPress(KeyEvent.VK_C);
%             prevKey = KeyEvent.VK_C;
%         elseif strcmp(curClass, 'null') && ~strcmp(prevClass, 'null')
%             mouse.keyRelease(prevKey);
%             prevKey = KeyEvent.VK_0;
%         end
    end

% --- Executes on button press in trainBtn.
function trainBtn_Callback(hObject, eventdata, handles)
% hObject    handle to trainBtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global signals baselineThresh

% cNames = {'left' 'right' 'forward' 'takeoff'};
cNames = {'up' 'down'};
feats = [];
trClass = cell(0,1);
winSize = 200;
winStep = 100;

for c = 1:length(signals)
    signal = signals{c};
    
    for i=1:length(signal)
        disp([cNames{c} ': ' num2str(i)']);
        s = double(signal{i});
        numChannels = size(s,2);
        
        for j=winSize+7:winStep:length(s)
            %Detrend window
            X = zeros(winSize,2);
            for k=j-winSize+1:j
                X(k-j+winSize,:) = s(k,:) - sum(s(k-7:k,:))/8;
            end
            
            %Extract features
            numFeat = 4;
            feat = zeros(1,numChannels*numFeat);
            rms = sqrt(mean(X.^2)); %RMS
            wl = sum(abs(diff(X))); %waveform length

            for k=1:numChannels
                offset = numFeat*(k-1);
                feat(offset+1) = rms(k);
                feat(offset+2) = wl(k);

                for l=1:length(X)-1
                    x = X(l,k);
                    xf = X(l+1,k);

                    %Trial Zero Crossings
                    if ((x>0 && xf<0) || (x<0 && xf>0))% && abs(x-xf)>=.01
                        feat(offset+3) = feat(offset+3)+1;
                    end

                    %Trial Slope Sign Changes
                    if l>1
                        xp = X(l-1,k);
                        if ((x>xp && x>xf) || (x<xp && x<xf))% && (abs(x-xf)>=.01 || abs(x-xp)>=.01)
                            feat(offset+4) = feat(offset+4)+1;
                        end
                    end
                end
            end

            feats = [feats; feat];
            trClass{end+1,1} = cNames{c};
            
        end
    end
end

lda = classify(feats,feats,trClass);
cMatLDA = confusionmat(trClass,lda)
save('lda.mat','feats','trClass','baselineThresh');


% --- Executes on button press in flyBtn.
function flyBtn_Callback(hObject, eventdata, handles)
% hObject    handle to flyBtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of flyBtn
global fly prevKey fclass trFeats trClass baselineThresh mouse curHandles

btnState = get(hObject,'Value');
if btnState
    load lda
    trFeats = feats;
    fly = true;
    prevKey = java.awt.event.KeyEvent.VK_0;
    mouse = java.awt.Robot;
    fclass = ones(1,10);
else
    fly = false;
    set(curHandles.statusText,'String', '');
end



function baseMultText_Callback(hObject, eventdata, handles)
% hObject    handle to baseMultText (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of baseMultText as text
%        str2double(get(hObject,'String')) returns contents of baseMultText as a double


% --- Executes during object creation, after setting all properties.
function baseMultText_CreateFcn(hObject, eventdata, handles)
% hObject    handle to baseMultText (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

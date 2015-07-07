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

% Last Modified by GUIDE v2.5 10-Feb-2015 17:37:41

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

if ~exist('ARDrone','class')
    errordlg('The class \"ARDrone\" was not found in the MATLAB path.',...
             'Class not found', 'modal');
end

global h cls fly baselineThresh ...
       butterC1 butterC2 Fs Hd_notch H Nsh L

Fs = 960;   % Sample rate


%%%%% FILTER STUFF
% [butterC1,butterC2]=butter(5,2*pi*(6/Fs),'high'); %defines 5th order Butterworth filter with 6Hz cutoff frequency
[butterC1,butterC2]=butter(5,[2*pi*(0.1/Fs), 2*pi*(10/Fs)]); %defines 5th order Butterworth filter with 0.1- 10Hz cutoff frequency

linenoise = 60;
L = Fs/linenoise;
Nsh = 15;
d = fdesign.comb('notch','L,BW,GBW,Nsh',L,3,-35,Nsh,Fs);
H = design(d);

 % All frequency values are in Hz.

% Fpass1 = 50;          % First Passband Frequency
% Fstop1 = 59;          % First Stopband Frequency
% Fstop2 = 61;          % Second Stopband Frequency
% Fpass2 = 70;          % Second Passband Frequency
% Apass1 = 0.5;         % First Passband Ripple (dB)
% Astop  = 60;          % Stopband Attenuation (dB)
% Apass2 = 1;           % Second Passband Ripple (dB)
% match  = 'stopband';  % Band to match exactly
% 
% % Construct an FDESIGN object and call its BUTTER method.
% tf  = fdesign.bandstop(Fpass1, Fstop1, Fstop2, Fpass2, Apass1, Astop, ...
%                       Apass2, Fs);
% Hd_notch = design(tf, 'butter', 'MatchExactly', match);

global pathToBioRadioDLL pathToConfigFile daq_type
global drone flying classes

pathToBioRadioDLL = 'C:\Users\kellygs\Dropbox\BioRadio Dev Stuff\drone_emg\BioRadioSDK\Matlab SDK';
pathToConfigFile  = fullfile(pathToBioRadioDLL,'ExampleConfig.ini');
daq_type = 'bioRadio';
drone = ARDrone;
% classes = {'Null','Takeoff/Land','Left','Right','Forward'};
classes = {'Null','Takeoff/Land','Right','Forward'};
set(handles.actionMenu,'String',classes(2:end));
flying = 0;

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
global s data time curHandles ch1plot ch2plot daq_type nullStarted timeNullStarted trialStarted
global signals non_filtData wls
global bioRadioHandle samplingTimer pathToBioRadioDLL pathToConfigFile

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
    
    switch daq_type
        case 'session'
            s = daq.createSession('ni');
            s.addAnalogInputChannel('Dev3',0,'Voltage');
            s.addAnalogInputChannel('Dev3',1,'Voltage');
            s.IsContinuous = true;
            s.addlistener('DataAvailable', @getMoreData);
            s.NotifyWhenDataAvailableExceeds = s.Rate/10; %getMoreData runs every .1s
            s.startBackground();
        case 'bioRadio'
            % Load the BioRadio150 SDK
            bioRadioHandle = BioRadio150_Load(pathToBioRadioDLL,false);
            % Find devices, and use the first one found
            deviceComPorts = BioRadio150_Find;
            if (length(deviceComPorts) <= 0)
                disp('*BioRadio*  no devices found; aborting');
                BioRadio150_Unload(bioRadioHandle);
                return;
            end
            portName = char(deviceComPorts(1));
            fprintf('*BioRadio*  using %s', portName);
    
            % Create timer object for periodic sampling
            samplingTimer = timer('StartDelay',0,...
                                  'Period',0.08,...
                                  'TasksToExecute',Inf,...
                                  'ExecutionMode','fixedSpacing',...
                                  'TimerFcn',{@getMoreData},...
                                  'StopFcn',@timerStopFcn);

            % Start data acquisition
            try
                BioRadio150_Start(bioRadioHandle,portName,true,pathToConfigFile);
                pause(0.1);
                start(samplingTimer);
            catch err
                BioRadio150_Unload(bioRadioHandle);
                rethrow(err);
            end
        case 'legacyAnalog'
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
    switch daq_type
        case 'session'
            s.stop();
            delete(s);
        case 'bioRadio'
            stop(samplingTimer);
            BioRadio150_Stop(bioRadioHandle);
            BioRadio150_Unload(bioRadioHandle);
        case 'legacyAnalog'
            stop(s);
    end
    set(hObject,'String', 'Start DAQ');
end

function getMoreData(src,event)
    global data time curHandles daq_type ch1plot ch2plot
    global nullStarted timeNullStarted nullDuration baselineThresh
    global trialStarted recordStart trialData
    global signals
    global fly fclass curClass trFeats trClass prevKey mouse h
    global Nsh L H butterC1 butterC2 non_filtData
    
    global BioRadio150_numEnabledFastInputs BioRadio150_fastDataReadSize 
    global flying bioRadioHandle drone classes Hd_notch
    
    %%
    % PLOTTING THE DATA
    %
    
    
    try
        ch1axes = curHandles.axes1;
        ch2axes = curHandles.axes2;

        switch daq_type
            case 'session'
                newData = event.Data;
                newTime = event.TimeStamps;
                rate = src.Rate;
            case 'bioRadio'
                [fastData,~] = BioRadio150_Read(bioRadioHandle);
                if BioRadio150_fastDataReadSize <= 0                   
                    return;
                end
                numSamples = BioRadio150_fastDataReadSize/BioRadio150_numEnabledFastInputs;
                newData = fastData(:,1:numSamples)';
                rate = 960;
                if isempty(time)
                    startTime = 0;
                else
                    startTime = time(end)+1/rate;
                end
                endTime = startTime+((size(newData,1)-1)/rate);
                newTime = (startTime:1/rate:endTime)';
            case 'legacyAnalog'
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
%         try
%             newData = filtfilt(butterC1,butterC2,newData); % Highpass
%         catch err
%             newData = [];
%             disp(err.message);
%             return;
%         end      
        
        

        %OLD STUFF
        non_filtData = [non_filtData; newData];
        time = [time; newTime];

        % Keep the last 5s of data
        if isempty(time) || newTime(end) < 5

            %Plots EMG if not flying
            set(ch1axes,'XLim',[time(1) time(end)],'XTick', time(1):.5:time(end),'YLim',[-0.000835 0.000835]);
            set(ch2axes,'XLim',[time(1) time(end)],'XTick', time(1):.5:time(end),'YLim',[-0.000835 0.000835]);
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
            try
                currData = [non_filtData(end-(amountToUse-size(newData,1))+1:end,:); newData]; %X
            catch err
                disp(err.message);
                return;
            end

            %Step 4: Comb the ~500 ms of data and take only 100 (inner) ms of it
            curr_filtData = filtfilt(H.Numerator, H.Denominator, padarray(currData,[padding 0])); %Xfilt
            curr_filtData = curr_filtData(end-padding-200+1:end-padding-100,:);                        %Xfilt

            data = [data; curr_filtData];
            
            if max(curr_filtData(:)) > 10e20
                disp('Whoa now')
            end

            %Plots EMG if not flying
            set(ch1axes,'XLim',[time(end-200+1) time(end-100)],'XTick', time(end-200+1):.5:time(end-100),'YLim',[-0.000835 0.000835]);
            set(ch2axes,'XLim',[time(end-200+1) time(end-100)],'XTick', time(end-200+1):.5:time(end-100),'YLim',[-0.000835 0.000835]);
            set(ch1plot,'xdata',time(end-200+1:end-100),'ydata',curr_filtData(:,1));
            set(ch2plot,'xdata',time(end-200+1:end-100),'ydata',curr_filtData(:,2),'Color',[0 .5 0]);
            
%             set(ch1axes,'XLim',[time(end-100) time(end)],'XTick', time(end-100):.5:time(end),'YLim',[-0.000835 0.000835]);
%             set(ch2axes,'XLim',[time(end-100) time(end)],'XTick', time(end-100):.5:time(end),'YLim',[-0.000835 0.000835]);
%             set(ch1plot,'xdata',time(end-100:end),'ydata',curr_filtData(:,1));
%             set(ch2plot,'xdata',time(end-100:end),'ydata',curr_filtData(:,2),'Color',[0 .5 0]);

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
            baselineThresh = baseMult*sqrt(mean(nullData.^2));
            disp(['Baseline Threshold: ' num2str(baselineThresh)]);
            timeNullStarted = 0;
            beep;
        end


        %%
        % GET DATA FOR TRIAL
        %
        
        if trialStarted && ~isempty(baselineThresh)
%             rms = sqrt(mean(data(end-.200*rate+1:end,:).^2));
            rms = mean(abs(data(end-.200*rate+1:end,:)));
            actionNum = get(curHandles.actionMenu, 'Value')+1;
            if ~recordStart
                set(curHandles.statusText,'String', 'Waiting...');
                if sum(rms > baselineThresh) > 0
                    set(curHandles.statusText,'String', 'Recording...');
                    disp(rms);
                    beep;
                    recordStart = true;
                    trialData = [];
                end
            else
                trialData = [trialData; newData];
                if sum(rms > baselineThresh) == 0 %Stop recording
                    beep;
                    recordStart = false;
                    trialNum = str2double(get(curHandles.trialNumText,'String'));
                    disp(['Trial ' num2str(trialNum)])
                    signals{actionNum}{trialNum} = trialData;
                    trialData = [];
                    if trialNum < 10
                        set(curHandles.trialNumText,'String',num2str(trialNum+1));
                    else
                        set(curHandles.trialNumText,'String','1');
                        set(curHandles.statusText,'String', '');
                        trialStarted = false;
                    end
                end
            end
        end

        if size(data,1) > .200*rate && fly

            signal = data(end-round(.200*rate)+1:end,:);
            fclass = circshift(fclass,[1 -1]);
            numChannels = size(signal,2);

            %Detrend
            X = zeros(length(signal)-7,numChannels);
            for j=8:length(signal)
                X(j-7,:) = signal(j,:) - mean(signal(j-7:j,:));
            end
            
%             save('signal.mat','X')
            cNames = classes;

            %Extract features
            numFeat = 4;
            feat = zeros(1,numChannels*numFeat);
            rms = sqrt(mean(X.^2)); %RMS

            if sum(rms > baselineThresh)==0
                fclass(10) = 1;
            else
                wl = sum(abs(diff(X))); % Waveform length
                zc = sum(diff(X>0)~=0); % # of zero crossings
                ssc = sum(diff(sign(diff(X)))~=0); % # of slope sign changes
 
                for k=1:numChannels
                    offset = numFeat*(k-1);
                    feat(offset+1) = rms(k);
                    feat(offset+2) = wl(k);
                    feat(offset+3) = zc(k);
                    feat(offset+4) = ssc(k);
                end

                fcName = classify(feat,trFeats,trClass,'mahalanobis');
                fclass(10) = find(strcmp(cNames,fcName(1)));
                disp(fclass)
            end 
            
            prevClass = curClass;
            if fclass(9)==1 && fclass(10)==1
                curClass = 'Null'; %null
                disp(curClass)
            else
                %Get majority class
                curClass = cNames{mode(fclass)};
                disp(curClass)
            end
            
            set(curHandles.statusText,'String',curClass);
            if strcmp(curClass, 'Takeoff/Land') && ~strcmp(prevClass, 'Takeoff/Land')
                if(flying==0)
                    drone.takeoff;
                    drone.takeoff;
                    flying=1;
                else
                    drone.land;
                    flying=0;
                end
            elseif strcmp(curClass, 'Forward')
                drone.moveForward(0.5);
%             elseif strcmp(curClass, 'Left') && ~strcmp(prevClass, 'Left')
%                 drone.moveForward(10);
            elseif strcmp(curClass, 'Right') && ~strcmp(prevClass, 'Right')
                drone.rotateRight(10);
            elseif strcmp(curClass, 'Null')
                drone.hover;
            end
        end
    catch err
        disp(err.stack.name(2));
        disp(err.stack.line(2));
    end

% --- Executes on button press in trainBtn.
function trainBtn_Callback(hObject, eventdata, handles)
% hObject    handle to trainBtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global signals baselineThresh
global classes

% cNames = {'left' 'right' 'forward' 'takeoff'};
cNames = classes;
feats = [];
trClass = cell(0,1);
winSize = 200;
winStep = 100;

for c = 1:length(signals)
    signal = signals{c};
    
    for i=1:length(signal)
        disp([cNames{c} ': ' num2str(i)]);
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
            zc = sum(diff(X>0)~=0); % # of zero crossings
            ssc = sum(diff(sign(diff(X)))~=0); % # of slope sign changes

            for k=1:numChannels
                offset = numFeat*(k-1);
                feat(offset+1) = rms(k);
                feat(offset+2) = wl(k);
                feat(offset+3) = zc(k);
                feat(offset+4) = ssc(k);               
            end

            feats = [feats; feat];
            trClass{end+1,1} = cNames{c};
            
        end
    end
end

lda = classify(feats,feats,trClass,'mahalanobis');
cMatLDA = confusionmat(trClass,lda);
disp(cMatLDA);
save('lda.mat','signals','feats','trClass','baselineThresh','cMatLDA');


% --- Executes on button press in flyBtn.
function flyBtn_Callback(hObject, eventdata, handles)
% hObject    handle to flyBtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of flyBtn
global fly prevKey fclass trFeats trClass baselineThresh mouse curHandles
global drone cls flying

btnState = get(hObject,'Value');
if btnState
%     x=load('Classifier.mat');
%     cls = x.cls;
    load('lda.mat');
    trFeats = feats;
    fly = true;
    fclass = ones(1,10);
else
    fly = false;
    set(curHandles.statusText,'String', '');
    if flying == 1
        try
            drone.land;
        catch err
            beep;
            beep;
            beep;
            disp('ERROR! Unable to land drone!')
            disp(err);
        end
    end
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

function timerStopFcn(mTimer,~)
delete(mTimer);
